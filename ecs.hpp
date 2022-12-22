#pragma once

#include <vector>
#include <unordered_map>
#include <tuple>
#include <memory>
#include <functional>
#include <cassert>

namespace ecs {

struct Resource {
    Resource() = default;
    Resource(const Resource&) = default;
    virtual ~Resource() = default;
};

struct Component {
    Component() = default;
    Component(const Component&) = default;
    virtual ~Component() = default;
};

template <typename T>
struct IndexGetter final {
    template <typename U>
    static uint32_t Get() {
        static uint32_t idx = curIdx_ ++;
        return idx;
    }

private:
    inline static uint32_t curIdx_ = 0;
};

template <typename T>
struct IDGenerator final {
    static uint32_t Gen() {
        static uint32_t curId_ = 0;
        return curId_ ++;
    }
};

template <typename T>
class ObjectPool {
public:
    using Elem = std::unique_ptr<T>;

    static ObjectPool& Instance() {
        static ObjectPool instance;
        return instance;
    }
    
    template <typename... Args>
    T* New(Args&&... args) {
        if (cache_.empty()) {
            datas_.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        } else {
            Elem elem = std::move(cache_.back());
            cache_.pop_back();
            new (elem.get()) T(std::forward<Args>(args)...);
            datas_.push_back(std::move(elem));
        }
        return datas_.back().get();
    }

    void Delete(T* elem) {
        if (auto it = std::find_if(datas_.begin(), datas_.end(),
                                   [&](const Elem& e) { return e.get() == elem; });
            it != datas_.end()) {
            cache_.push_back(std::move(*it));
            datas_.erase(it);
        }
    }

private:
    std::vector<Elem> datas_;
    std::vector<Elem> cache_;
};

using ComponentContainer = std::unordered_map<uint32_t, Component*>;
using ResourceContainer = std::unordered_map<uint32_t, Resource*>;
using Entity = uint32_t;
using EntityContainer = std::unordered_map<Entity, ComponentContainer>;

class Queryer;
class Commands;
class Resources;

using StartSystem = std::function<void(Commands&)>;
using UpdateSystem = std::function<void(Commands&, Queryer&, Resources&)>;

class Storage final {
public:
    friend class Resources;
    friend class Commands;
    friend class Queryer;

    Storage& AddStartSystem(StartSystem sys) {
        startSystems_.push_back(sys);
        return *this;
    }

    Storage& AddUpdateSystem(UpdateSystem sys) {
        updateSystems_.push_back(sys);
        return *this;
    }

    template <typename T, typename... Args>
    Storage& SetResource(Args&&... args);

    void Startup();
    void Update();

private:
    EntityContainer entities_;
    ResourceContainer resources_;
    std::vector<StartSystem> startSystems_;
    std::vector<UpdateSystem> updateSystems_;
};

class Queryer final {
public:
    template <typename... Components>
    class Result final {
    public:
        friend class Queryer;

        template <typename T>
        T* Get() {
            return std::get<T*>(datas_);
        }

    private:
        std::tuple<Components...> datas_;
    };

    Queryer(Storage& storage): storage_(storage) {}

    template <typename... Args>
    using ResultType = Result<Args*...>;

    template <typename... Args>
    std::vector<ResultType<Args...>> Query() {
        std::vector<ResultType<Args...>> results;
        for (auto it = storage_.entities_.begin(); it != storage_.entities_.end(); it++) {
            ResultType<Args...> result;
            if (addSatisfiedComponents<Args...>(result, it->second)) {
                results.push_back(std::move(result));
            }
        }
        return results;
    }

private:
    Storage& storage_;

    template <typename... Args>
    bool addSatisfiedComponents(ResultType<Args...>& result, ComponentContainer& components) {
        AddComponentsHelper<Args...> helper;
        return helper(result, components);
    }

    template <typename... Args>
    struct AddComponentsHelper final {
        using ResultT = ResultType<Args...>;

        bool operator()(ResultT& result, ComponentContainer& components) {
            return addComponentsRecur<Args...>(result, components);
        }

        template <typename T, typename... Remains>
        bool addComponentsRecur(ResultT& result, ComponentContainer& components) {
            auto idx = IndexGetter<Component>::Get<T>();
            if (auto it = components.find(idx); it != components.end()) {
                std::get<T*>(result.datas_) = static_cast<T*>(it->second);
                if constexpr (sizeof...(Remains) != 0) {
                    return addComponentsRecur<Remains...>(result, components);
                } else {
                    return true;
                }
            }
            return false;
        }
    };

};

class Commands final {
public:
    Commands(Storage& storage): storage_(storage) {}

    Commands(Commands&& other): storage_(other.storage_) {
        newResources_ = std::move(other.newResources_);
        newEntities_ = std::move(other.newEntities_);
    }

    template <typename... Args>
    Commands& Spawn(Args&&... args) {
        auto id = IDGenerator<Entity>::Gen();
        ComponentContainer container;
        saveSpawnComponents<Args...>(container, std::forward<Args>(args)...);
        newEntities_[id] = std::move(container);
        return *this;
    }

    template <typename T, typename... Args>
    Commands& SetResource(Args&&... args) {
        static_assert(std::is_base_of_v<Resource, T>, "your resource must inherit from ecs::Resource");
        auto index = IndexGetter<Resources>::Get<T>();
        assert(("resource already exists", resources_.find(index) == resources_.end()));
        newResources_[index] = std::unique_ptr<T>(new T(std::forward(args)...));
        return *this;
    }

    template <typename T, typename... Args>
    Commands& ReplaceResource(Args&&... args) {
        newResources_[IndexGetter<Resource>::Get<T>()] = std::unique_ptr<T>(new T(std::forward(args)...));
        return *this;
    }

    void Execute() {
        for (auto& entity : newEntities_) {
            storage_.entities_.emplace(entity.first, std::move(entity.second));
        }
        newEntities_.clear();
        for (auto& resource : newResources_) {
            storage_.resources_.emplace(resource.first, std::move(resource.second));
        }
        newResources_.clear();
    }

private:
    Storage& storage_;
    ResourceContainer newResources_;
    EntityContainer newEntities_;

    template <typename T, typename... Args>
    void saveSpawnComponents(ComponentContainer& container, T&& t, Args&&... args) {
        static_assert(std::is_base_of_v<Component, T>, "your component must inherit from ecs::Component");
        container[IndexGetter<Component>::Get<T>()] = ObjectPool<T>::Instance().New(std::forward<T>(t));
        saveSpawnComponents<Args...>(container, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    void saveSpawnComponents(ComponentContainer& container, T&& t) {
        static_assert(std::is_base_of_v<Component, T>, "your component must inherit from ecs::Component");
        container[IndexGetter<Component>::Get<T>()] = ObjectPool<T>::Instance().New(std::forward<T>(t));
    }
};

class Resources final {
public:
    Resources(Storage& storage): storage_(storage) {}

    template <typename T>
    T* Get() {
        auto index = IndexGetter<Resources>::Get<T>();
        if (auto it = storage_.resources_.find(index); it != storage_.resources_.end()) {
            return static_cast<T*>(it->second);
        }
        return nullptr;
    }

private:
    Storage& storage_;
};


/********* implementation of Storage ************/

void Storage::Startup() {
    for (auto& system : startSystems_) {
        Commands cmds(*this);
        system(cmds);
        cmds.Execute();
    }
}

void Storage::Update() {
    static std::vector<Commands> commandList;
    commandList.clear();
    for (auto& system : updateSystems_) {
        Commands cmds(*this);
        Queryer queyer(*this);
        Resources resources(*this);
        system(cmds, queyer, resources);
        commandList.emplace_back(std::move(cmds));
    }

    for (auto& cmd : commandList) {
        cmd.Execute();
    }
}

template <typename T, typename... Args>
Storage& Storage::SetResource(Args&&... args) {
    static_assert(std::is_base_of_v<Resource, T>, "your resource must inherit from ecs::Resource");
    auto index = IndexGetter<Resources>::Get<T>();
    assert(("resource already exists", resources_.find(index) == resources_.end()));
    resources_[index] = ObjectPool<T>::Instance().New(std::forward(args)...);
    return *this;
}

}