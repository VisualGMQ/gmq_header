#include "ecs.hpp"
#include <string>
#include <iostream>

using namespace ecs;

struct Name {
    std::string name;
};

struct ID {
    int id;
};

struct Timer {
    int t;
};

void StartUpSystem(Commands& command) {
    command.Spawn<Name>(Name{ "person1" })
           .Spawn<Name, ID>(Name{"person2"}, ID{1})
           .Spawn<ID>(ID{2});
}

void EchoNameSystem(Commands& command, Queryer query, Resources resources, Events& e) {
    std::cout << "echo name system" << std::endl;
    std::vector<Entity> entities = query.Query<Name>();
    for (auto entity : entities) {
        std::cout << query.Get<Name>(entity).name << std::endl;
    }

    auto reader = e.Reader<std::string>();
    if (reader.Has()) {
        std::cout << reader.Read() << std::endl;
    }
}

void EchoNameAndIDSystem(Commands& command, Queryer query, Resources resources, Events& e) {
    std::cout << "echo name and id system" << std::endl;
    std::vector<Entity> entities = query.Query<Name, ID>();
    for (auto entity : entities) {
        std::cout << query.Get<Name>(entity).name << ", " << query.Get<ID>(entity).id << std::endl;
    }

    auto reader = e.Reader<std::string>();
    if (reader.Has()) {
        std::cout << reader.Read() << std::endl;
    }
}

bool canWrite = true;

void EchoIDSystem(Commands& command, Queryer query, Resources resources, Events& e) {
    std::cout << "echo id system" << std::endl;
    std::vector<Entity> entities = query.Query<ID>();
    for (auto entity : entities) {
        std::cout << query.Get<ID>(entity).id << std::endl;
    }

    if (canWrite) {
        e.Writer<std::string>().Write("hello, I'm EchoIDSystem");
        canWrite = false;
    }
}

void EchoTimeSystem(Commands& command, Queryer query, Resources resources, Events& e) {
    std::cout << "echo time system" << std::endl;
    std::cout << resources.Get<Timer>().t << std::endl;

    auto reader = e.Reader<std::string>();
    if (reader.Has()) {
        std::cout << reader.Read() << std::endl;
    }
}

int main() {
    World world;
    world.AddStartupSystem(StartUpSystem)
         .SetResource<Timer>(Timer{123})
         .AddSystem(EchoNameSystem)
         .AddSystem(EchoNameAndIDSystem)
         .AddSystem(EchoIDSystem)
         .AddSystem(EchoTimeSystem);

    world.Startup();

    world.Update();
    std::cout << "==================" << std::endl;
    world.Update();
    std::cout << "==================" << std::endl;
    world.Update();

    world.Shutdown();
    return 0;
}
