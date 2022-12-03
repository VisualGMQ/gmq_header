#include "cgmath.hpp"

#define CATCH_CONFIG_MAIN
#include "3rdlibs/catch.hpp"

#include <limits>

#define FLT_EQ(a, b) (std::abs(a - b) <= std::numeric_limits<float>::epsilon())

TEST_CASE("vectors can be add, sub, mul and divide", "[vector]") {
    cgmath::Vec2 v1(1, 2);
    cgmath::Vec2 v2(2, 3);
    
    SECTION("vector add") {
        auto result = v1 + v2;
        REQUIRE(result.x == 3);
        REQUIRE(result.y == 5);
    }

    SECTION("vector sub") {
        auto result = v1 - v2;
        REQUIRE(result.x == -1);
        REQUIRE(result.y == -1);
    }

    SECTION("vector mul") {
        auto result = v1 * v2;
        REQUIRE(result.x == 2);
        REQUIRE(result.y == 6);
    }

    SECTION("vector div") {
        auto result = v1 / v2;
        REQUIRE(result.x == 0.5);
        REQUIRE(result.y == 2.0f / 3.0f);
    }

    SECTION("vector mul scalar") {
        auto result = v1 * 0.5;
        REQUIRE(result.x == 0.5);
        REQUIRE(result.y == 1);
        result = 0.5 * v1;
        REQUIRE(result.x == 0.5);
        REQUIRE(result.y == 1);
    }
}

TEST_CASE("vectors can be dot", "[vector]") {
    SECTION("vector dot") {
        auto result = cgmath::Vec2(1, 2).Dot(cgmath::Vec2(2, 3));
        REQUIRE(result == 8);
    }
}

TEST_CASE("Vec2 and Vec3 can be cross", "[vector]") {
    SECTION("Vec2 cross") {
        auto result = cgmath::Vec2(1, 2).Cross(cgmath::Vec2(2, 3));
        REQUIRE(result == -1);
    }

    SECTION("Vec3 cross") {
        auto result = cgmath::Vec3(1, 2, 3).Cross(cgmath::Vec3(4, 5, 6));
        REQUIRE(result.x == -3);
        REQUIRE(result.y == 6);
        REQUIRE(result.z == -3);
    }
}

TEST_CASE("vectors can get length and normalize", "[vector]") {
    SECTION("vector length") {
        cgmath::Vec2 v1(1, 2);
        cgmath::Vec2 v2(2, 3);
        REQUIRE(v1.LengthSquare() == 5);
        REQUIRE(v2.LengthSquare() == 13);
        REQUIRE(FLT_EQ(v1.Length(), std::sqrt(5)));
        REQUIRE(FLT_EQ(v2.Length(), std::sqrt(13)));
    }

    SECTION("vector normalize") {
        cgmath::Vec2 v1(1, 2);
        float len = v1.Length();
        v1.Normalize();
        REQUIRE(v1.x == 1 / len);
        REQUIRE(v1.y == 2 / len);

        auto v2 = cgmath::Normalize(cgmath::Vec2(1, 2));
        REQUIRE(v2.x == 1 / len);
        REQUIRE(v2.y == 2 / len);
    }
}

TEST_CASE("vectors can be assigned", "[vector]") {
    cgmath::Vec2 v1(1, 2);
    cgmath::Vec2 v2 = v1;
    v1.x = 3;
    REQUIRE(v1.x == 3);
    REQUIRE(v2.x == 1);
    v2 = v1;
    v1.x = 6;
    REQUIRE(v2.x == 3);
    REQUIRE(v1.x == 6);
}