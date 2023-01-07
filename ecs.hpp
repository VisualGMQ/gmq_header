#pragma once
#include <vector>
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <functional>
#include "sparse_sets.hpp"

#define assertm(msg, expr) assert(((void)msg, (expr)))

namespace ecs {

using ComponentID = uint32_t;
using Entity  = uint32_t;

struct Resource{};
struct Component{};

template <typename Category>
class IndexGetter final {
public:
    template <typename T>
    static uint32_t Get() {
        static uint32_t id = curIdx_ ++;
        return id;
    }

private:
    inline static uint32_t curIdx_ = 0;
};

template <typename T, typename = std::enable_if<std::is_integral_v<T>>>
struct IDGenerator final {
public:
    static T Gen() {
        return curId_ ++;
    }

private:
    inline static T curId_ = {};
};

using EntityGenerator = IDGenerator<Entity>;

class Commands;
class Resources;
class Queryer;

using UpdateSystem = void(*)(Commands&, Queryer, Resources);
using StartupSystem = void(*)(Commands&);

class World final {
public:
    friend class Commands;
    friend class Resources;
    friend class Queryer;
    using ComponentContainer = std::unordered_map<ComponentID, void*>;

    World() = default;
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    World& AddStartupSystem(StartupSystem sys) {
        startupSystems_.push_back(sys);

        return *this;
    }

    World& AddSystem(UpdateSystem sys) {
        updateSystems_.push_back(sys);

        return *this;
    }

    template <typename T>
    World& SetResource(T&& resource);

    void Startup();
    void Update();
    void Shutdown() {
        entities_.clear();
        resources_.clear();
        componentMap_.clear();
    }

private:
    struct Pool final {
        std::vector<void*> instances;
        std::vector<void*> cache;

        using CreateFunc = void*(*)(void);
        using DestroyFunc = void(*)(void*);

        CreateFunc create;
        DestroyFunc destroy;
        
        Pool(CreateFunc create, DestroyFunc destroy): create(create), destroy(destroy) {
            assertm("you must give a non-nullptr create func", create);
            assertm("you must give a non-nullptr destroy func", create);
        }

        void* Create() {
            if (!cache.empty()) {
                instances.push_back(cache.back());
                cache.pop_back();
            } else {
                instances.push_back(create());
            }
            return instances.back();
        }

        void Destroy(void* elem) {
            if (auto it = std::find(instances.begin(), instances.end(), elem);
                it != instances.end()) {
                cache.push_back(*it);
                std::swap(*it, instances.back());
                instances.pop_back();
            } else {
                assertm("your element not in pool", false);
            }
        }
    };

    struct ComponentInfo {
        Pool pool;
        SparseSets<Entity, 32> sparseSet;

        ComponentInfo(Pool::CreateFunc create, Pool::DestroyFunc destroy): pool(create, destroy) {}
        ComponentInfo(): pool(nullptr, nullptr) {}
    };

    using ComponentMap = std::unordered_map<ComponentID, ComponentInfo>; 
    ComponentMap componentMap_;
    std::unordered_map<Entity, ComponentContainer> entities_;

    struct ResourceInfo {
        void* resource = nullptr;
        using DestroyFunc = void(*)(void*);
        DestroyFunc destroy = nullptr;

        ResourceInfo(DestroyFunc destroy): destroy(destroy) {
            assertm("you must give a non-null destroy function", destroy);
        }
        ResourceInfo() = default;
        ~ResourceInfo() {
            destroy(resource);
        }
    };

    std::unordered_map<ComponentID, ResourceInfo> resources_;
    std::vector<StartupSystem> startupSystems_;
    std::vector<UpdateSystem> updateSystems_;
};

class Commands final {
public:
    Commands(World& world): world_(world) {}

    template <typename... ComponentTypes>
    Commands& Spawn(ComponentTypes&&... components) {
        SpawnAndReturn<ComponentTypes...>(std::forward<ComponentTypes>(components)...);
        return *this;
    }

    template <typename... ComponentTypes>
    Entity SpawnAndReturn(ComponentTypes&&... components) {
        EntitySpawnInfo info;
        info.entity = EntityGenerator::Gen();
        doSpawn<ComponentTypes...>(info.entity, info.components, std::forward<ComponentTypes>(components)...);
        return info.entity;
    }

    Commands& Destroy(Entity entity) {
        destroyEntities_.push_back(entity);

        return *this;
    }

    template <typename T>
    Commands& SetResource(T&& resource) {
        auto index = IndexGetter<Resource>::Get<T>(); 
        if (auto it = world_.resources_.find(index); it != world_.resources_.end()) {
            assertm("resource already exists", it->second.resource);
            it->second.resource = new T(std::forward<T>(resource));
        } else {
            auto newIt = world_.resources_.emplace(index, World::ResourceInfo([](void* elem) {delete (T*)elem; }));
            newIt.first->second.resource = new T;
            *(T*)(newIt.first->second.resource) = std::forward<T>(resource);
        }

        return *this;
    }

    template <typename T>
    Commands& RemoveResource() {
        auto index = IndexGetter<Resource>::Get<T>(); 
        destroyResources_.push_back(ResourceDestroyInfo(index, [](void* elem){ delete (T*)elem; }));

        return *this;
    }

    void Execute() {
        for (auto& info : destroyResources_) {
            removeResource(info);
        }
        for (auto e : destroyEntities_) {
            destroyEntity(e);
        }

        for (auto& spawnInfo : spawnEntities_) {
            auto it = world_.entities_.emplace(spawnInfo.entity, World::ComponentContainer{});
            auto& componentContainer = it.first->second;
            for (auto& componentInfo : spawnInfo.components) {
                componentContainer[componentInfo.index] = doSpawnWithoutType(spawnInfo.entity, componentInfo);
            }
        }
    }

private:
    World& world_;

    using DestroyFunc = void(*)(void*);

    struct ResourceDestroyInfo {
        uint32_t index;
        DestroyFunc destroy;

        ResourceDestroyInfo(uint32_t index, DestroyFunc destroy): index(index), destroy(destroy) {}
    };

    using AssignFunc = std::function<void(void*)>;

    struct ComponentSpawnInfo {
        AssignFunc assign;
        World::Pool::CreateFunc create;
        World::Pool::DestroyFunc destroy;
        ComponentID index;
    };

    struct EntitySpawnInfo {
        Entity entity;
        std::vector<ComponentSpawnInfo> components;
    };

    std::vector<Entity> destroyEntities_;
    std::vector<ResourceDestroyInfo> destroyResources_;
    std::vector<EntitySpawnInfo> spawnEntities_;

    template <typename T, typename... Remains>
    void doSpawn(Entity entity, std::vector<ComponentSpawnInfo>& spawnInfo, T&& component, Remains&&... remains) {
        ComponentSpawnInfo info;
        info.index = IndexGetter<Component>::Get<T>();
        info.create = [](void)->void* {
            return new T;
        };
        info.destroy = [](void* elem){
            delete (T*)elem;
        };
        info.assign = [&](void* elem) {
            static auto com = std::forward<T>(component);
            *((T*)elem) = com;
        };
        spawnInfo.push_back(info);

        if constexpr (sizeof...(Remains) != 0) {
            doSpawn<Remains...>(entity, spawnInfo, std::forward<Remains>(remains)...);
        }
    }

    void* doSpawnWithoutType(Entity entity, ComponentSpawnInfo& info) {
        if (auto it = world_.componentMap_.find(info.index); it == world_.componentMap_.end()) {
            world_.componentMap_.emplace(info.index,
                                         World::ComponentInfo(info.create, info.destroy));
        }
        World::ComponentInfo& componentInfo = world_.componentMap_[info.index];
        void* elem = componentInfo.pool.Create();
        info.assign(elem);
        componentInfo.sparseSet.Add(entity);

        return elem;
    }


    void destroyEntity(Entity entity) {
        if (auto it = world_.entities_.find(entity); it != world_.entities_.end()) {
            for (auto& [id, component] : it->second) {
                auto& componentInfo = world_.componentMap_[id];
                componentInfo.pool.Destroy(component);
                componentInfo.sparseSet.Remove(entity);
            }
            world_.entities_.erase(it);
        }
    }


    void removeResource(ResourceDestroyInfo& info) {
        if (auto it = world_.resources_.find(info.index); it != world_.resources_.end()) {
            info.destroy(it->second.resource);
            it->second.resource = nullptr;
        }
    }
};

class Resources final {
public:
    Resources(World& world): world_(world) {}

    template <typename T>
    bool Has() const {
        auto index = IndexGetter<Component>::Get<T>(); 
        auto it = world_.resources_.find(index);
        return it != world_.resources_.end() && it->second.resource;
    }

    template <typename T>
    T& Get() {
        auto index = IndexGetter<Resource>::Get<T>(); 
        return *((T*)world_.resources_[index].resource);
    }

private:
    World& world_;
};

class Queryer final {
public:
    Queryer(World& world): world_(world) {}
    
    template <typename... Components>
    std::vector<Entity> Query() const {
        std::vector<Entity> entities;
        doQuery<Components...>(entities);
        return entities;
    }

    template <typename T>
    bool Has(Entity entity) const {
        auto it = world_.entities_.find(entity);
        auto index = IndexGetter<Component>::Get<T>();
        return it != world_.entities_.end() && it->second.find(index) != it->second.end();
    }

    template <typename T>
    T& Get(Entity entity) {
        auto index = IndexGetter<Component>::Get<T>();
        return *((T*)world_.entities_[entity][index]);
    }

private:
    World& world_;

    template <typename T, typename... Remains>
    void doQuery(std::vector<Entity>& outEntities) const {
        auto index = IndexGetter<Component>::Get<T>();
        World::ComponentInfo& info = world_.componentMap_[index];

        for (auto e : info.sparseSet) {
            if constexpr (sizeof...(Remains) != 0) {
                doQueryRemains<Remains...>(e, outEntities);
            } else {
                outEntities.push_back(e);
            }
        }
    }

    template <typename T, typename... Remains>
    void doQueryRemains(Entity entity, std::vector<Entity>& outEntities) const {
        auto index = IndexGetter<Component>::Get<T>();
        auto& componentContainer = world_.entities_[entity];
        if (auto it = componentContainer.find(index); it == componentContainer.end()) {
            return ;
        }

        if constexpr (sizeof...(Remains) == 0) {
            outEntities.push_back(entity);
        } else {
            doQueryRemains<Remains...>(entity, outEntities);
        }
    }
};

inline void World::Startup() {
    std::vector<Commands> commandList;

    for (auto sys : startupSystems_) {
        Commands commands{*this};
        sys(commands);
        commandList.push_back(commands);
    }

    for (auto& commands : commandList) {
        commands.Execute();
    }
}

inline void World::Update() {
    std::vector<Commands> commandList;

    for (auto sys : updateSystems_) {
        Commands commands{*this};
        sys(commands, Queryer{*this}, Resources{*this});
        commandList.push_back(commands);
    }

    for (auto& commands : commandList) {
        commands.Execute();
    }
}

template <typename T>
World& World::SetResource(T&& resource) {
    Commands commands(*this);
    commands.SetResource(std::forward<T>(resource));

    return *this;
}

}
