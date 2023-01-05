#include "ecs.hpp"

#include <iostream>

struct GameState: public ecs::Resource {
    int count = 1;
};

struct Name: public ecs::Component {
    std::string name;

    Name(const std::string& name): name(name) {}
};

struct ID: public ecs::Component {
    uint32_t id;

    ID(uint32_t id): id(id) {}
};

void StartupSystem(ecs::Commands& cmds) {
    std::cout << "startup" << std::endl;
    cmds.Spawn(Name("ECSMan1"), ID(1))
        .Spawn(Name("ECSMan2"), ID(2));
}

void UpdateSystem(ecs::Commands& cmds, ecs::Queryer& queryer, ecs::Resources& resources) {
    std::cout << "hello" << std::endl;
    auto state = resources.Get<GameState>();
    assert(state);
    std::cout << state->count << std::endl;

    auto results = queryer.Query<Name, ID>();
    for (auto& result : results) {
        auto name = result.Get<Name>();
        auto id = result.Get<ID>();
        std::cout << result.Get<Name>()->name << "'s id = " << result.Get<ID>()->id << std::endl;
    }
}

int main() {
    ecs::World storage;
    storage.AddStartSystem(StartupSystem)
           .AddUpdateSystem(UpdateSystem)
           .SetResource<GameState>();

    storage.Startup();
    storage.Update();

    return 0;
}