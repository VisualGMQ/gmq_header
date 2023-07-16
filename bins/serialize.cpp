#include "serialize.hpp"

#define CATCH_CONFIG_MAIN
#include "3rdlibs/catch.hpp"

struct PersonConfig {
    std::string name;
    float height;
    bool female;
};

struct FamilyConfig {
    PersonConfig mother, father;
    std::vector<PersonConfig> children;
    std::string name;
    std::array<int, 4> ids;
};

struct IDNameMap{
    std::unordered_map<int, std::string> data;
};

ReflClass(PersonConfig) {
    Constructors()
    Fields(
        Field("name", &PersonConfig::name),
        Field("height", &PersonConfig::height),
        Field("female", &PersonConfig::female),
    )
};

ReflClass(FamilyConfig) {
    Constructors()
    Fields(
        Field("mother", &FamilyConfig::mother),
        Field("father", &FamilyConfig::father),
        Field("children", &FamilyConfig::children),
        Field("name", &FamilyConfig::name),
        Field("ids", &FamilyConfig::ids),
    )
};

ReflClass(IDNameMap) {
    Constructors()
    Fields(
        Field("data", &IDNameMap::data)
    )
};

TEST_CASE("deserialize", "[serialize]") {
    sol::state lua;
    lua.open_libraries(sol::lib::base);

    SECTION("deserialize plain data") {
        lua.do_string(R"(
        Config = {
            name = "VisualGMQ",
            height = 175.0,
            female = false,
        }
        )");
        sol::table table = lua["Config"];
        sol::object obj = table;
        std::optional<PersonConfig> config = serialize::DeserializeFromLua<PersonConfig>(table);
        REQUIRE(config.has_value());
        REQUIRE(config->name == "VisualGMQ");
        REQUIRE(config->height == 175.0);
        REQUIRE(config->female == false);
    }

    SECTION("deserialize class member") {
        lua.do_string(R"(
        Config = {
            mother = {
                name = "mother",
                height = 160.3,
                female = true,
            },
            father = {
                name = "father",
                height = 175.2,
                female = false,
            },
            children = {
                {
                    name = "child1",
                    height = 40.9,
                    female = false,
                },
                {
                    name = "child2",
                    height = 55.18,
                    female = true,
                },
            },
            name = "happy family",
            ids = { 1, 2, 3, 4 },
        }
        )");
        sol::table table = lua["Config"];
        std::optional<FamilyConfig> family = serialize::DeserializeFromLua<FamilyConfig>(table);
        REQUIRE(family.has_value());
        REQUIRE(family->mother.name == "mother");
        REQUIRE(family->mother.height == 160.3f);
        REQUIRE(family->mother.female == true);

        REQUIRE(family->father.name == "father");
        REQUIRE(family->father.height == 175.2f);
        REQUIRE(family->father.female == false);

        REQUIRE(family->children.size() == 2);

        REQUIRE(family->children[0].name == "child1");
        REQUIRE(family->children[0].height == 40.9f);
        REQUIRE(family->children[0].female == false);

        REQUIRE(family->children[1].name == "child2");
        REQUIRE(family->children[1].height == 55.18f);
        REQUIRE(family->children[1].female == true);

        REQUIRE(family->name == "happy family");
        REQUIRE(family->ids[0] == 1);
        REQUIRE(family->ids[1] == 2);
        REQUIRE(family->ids[2] == 3);
        REQUIRE(family->ids[3] == 4);
    }

    SECTION("deserialize unordered_map member") {
        lua.do_string(R"(
        Config = {
            data = {
                [3] = "elem3",
                [4] = "elem4",
                [6] = "elem6",
            }
        }
        )");
        sol::table table = lua["Config"];
        std::optional<IDNameMap> m = serialize::DeserializeFromLua<IDNameMap>(table);
        REQUIRE(m.has_value());
        REQUIRE(m.value().data[3] == "elem3");
        REQUIRE(m.value().data[4] == "elem4");
        REQUIRE(m.value().data[6] == "elem6");
    }
}