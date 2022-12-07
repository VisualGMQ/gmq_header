#include "cgmath.hpp"

#define CATCH_CONFIG_MAIN
#include "3rdlibs/catch.hpp"

#include <limits>

#define FLT_EQ(a, b) (std::abs(a - b) <= std::numeric_limits<float>::epsilon())

TEST_CASE("vectors can be add, sub, mul and divide", "[vector]") {
    cgmath::Vec2 v1{1, 2};
    cgmath::Vec2 v2{2, 3};
    
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
        auto result = cgmath::Vec2{1, 2}.Dot(cgmath::Vec2{2, 3});
        REQUIRE(result == 8);
    }
}

TEST_CASE("Vec2 and Vec3 can be cross", "[vector]") {
    SECTION("Vec2 cross") {
        auto result = cgmath::Vec2{1, 2}.Cross(cgmath::Vec2{2, 3});
        REQUIRE(result == -1);
    }

    SECTION("Vec3 cross") {
        auto result = cgmath::Vec3{1, 2, 3}.Cross(cgmath::Vec3{4, 5, 6});
        REQUIRE(result.x == -3);
        REQUIRE(result.y == 6);
        REQUIRE(result.z == -3);
    }
}

TEST_CASE("vectors can get length and normalize", "[vector]") {
    SECTION("vector length") {
        cgmath::Vec2 v1{1, 2};
        cgmath::Vec2 v2{2, 3};
        REQUIRE(v1.LengthSquare() == 5);
        REQUIRE(v2.LengthSquare() == 13);
        REQUIRE(FLT_EQ(v1.Length(), std::sqrt(5)));
        REQUIRE(FLT_EQ(v2.Length(), std::sqrt(13)));
    }

    SECTION("vector normalize") {
        cgmath::Vec2 v1{1, 2};
        float len = v1.Length();
        v1.Normalize();
        REQUIRE(v1.x == 1 / len);
        REQUIRE(v1.y == 2 / len);

        auto v2 = cgmath::Normalize(cgmath::Vec2{1, 2});
        REQUIRE(v2.x == 1 / len);
        REQUIRE(v2.y == 2 / len);
    }
}

TEST_CASE("vectors can be assigned", "[vector]") {
    cgmath::Vec2 v1{1, 2};
    cgmath::Vec2 v2 = v1;
    v1.x = 3;
    REQUIRE(v1.x == 3);
    REQUIRE(v2.x == 1);
    v2 = v1;
    v1.x = 6;
    REQUIRE(v2.x == 3);
    REQUIRE(v1.x == 6);
}

TEST_CASE("matrix can be add, sub, multiply and divide", "[matrix]") {
    /*
    [1, 2]
    [3, 4]
    */
    cgmath::Mat22 m1{1, 2,
                     3, 4};
    /*
    [5, 6]
    [7, 8]
    */
    cgmath::Mat22 m2({cgmath::Vec2{5, 7},
                      cgmath::Vec2{6, 8}});

    SECTION("matrix init") {
        REQUIRE(m1.Get(0, 0) == 1);
        REQUIRE(m1.Get(1, 0) == 2);
        REQUIRE(m1.Get(0, 1) == 3);
        REQUIRE(m1.Get(1, 1) == 4);

        REQUIRE(m2.Get(0, 0) == 5);
        REQUIRE(m2.Get(1, 0) == 6);
        REQUIRE(m2.Get(0, 1) == 7);
        REQUIRE(m2.Get(1, 1) == 8);
    }

    SECTION("matrix add") {
        auto m = m1 + m2;
        REQUIRE(m.Get(0, 0) == 6);
        REQUIRE(m.Get(1, 0) == 8);
        REQUIRE(m.Get(0, 1) == 10);
        REQUIRE(m.Get(1, 1) == 12);
    }

    SECTION("matrix sub") {
        auto m = m1 - m2;
        REQUIRE(m.Get(0, 0) == -4);
        REQUIRE(m.Get(1, 0) == -4);
        REQUIRE(m.Get(0, 1) == -4);
        REQUIRE(m.Get(1, 1) == -4);
    }   

    SECTION("matrix mul") {
        auto m = m1 * m2;
        REQUIRE(m.Get(0, 0) == 5);
        REQUIRE(m.Get(1, 0) == 12);
        REQUIRE(m.Get(0, 1) == 21);
        REQUIRE(m.Get(1, 1) == 32);
    }

    SECTION("matrix div") {
        auto m = m1 / m2;
        REQUIRE(m.Get(0, 0) == 1.0f / 5.0f);
        REQUIRE(m.Get(1, 0) == 2.0f / 6.0f);
        REQUIRE(m.Get(0, 1) == 3.0f / 7.0f);
        REQUIRE(m.Get(1, 1) == 4.0f / 8.0f);
    }    

    SECTION("matrix mul scalar") {
        auto m = m1 * 2;
        REQUIRE(m.Get(0, 0) == 2);
        REQUIRE(m.Get(1, 0) == 4);
        REQUIRE(m.Get(0, 1) == 6);
        REQUIRE(m.Get(1, 1) == 8);

        m = 2 * m1;
        REQUIRE(m.Get(0, 0) == 2);
        REQUIRE(m.Get(1, 0) == 4);
        REQUIRE(m.Get(0, 1) == 6);
        REQUIRE(m.Get(1, 1) == 8);
    }   

    SECTION("matrix multiply matrix") {
        auto m = m1.Mul(m2);
        REQUIRE(m.Get(0, 0) == 19);
        REQUIRE(m.Get(1, 0) == 22);
        REQUIRE(m.Get(0, 1) == 43);
        REQUIRE(m.Get(1, 1) == 50);

        auto m3 = cgmath::Matrix<float, 2, 3>{
            1, 2,
            3, 4,
            5, 6,
        };

        auto m4 = cgmath::Matrix<float, 3, 2>{
            1, 2, 3,
            4, 5, 6,
        };

        auto result = Mul(m3, m4);
        REQUIRE(result.W() == 3);
        REQUIRE(result.H() == 3);
        REQUIRE(result.Get(0, 0) == 9);
        REQUIRE(result.Get(1, 0) == 12);
        REQUIRE(result.Get(2, 0) == 15);
        REQUIRE(result.Get(0, 1) == 19);
        REQUIRE(result.Get(1, 1) == 26);
        REQUIRE(result.Get(2, 1) == 33);
        REQUIRE(result.Get(0, 2) == 29);
        REQUIRE(result.Get(1, 2) == 40);
        REQUIRE(result.Get(2, 2) == 51);
    }

    SECTION("matrix multiple vector") {
        cgmath::Mat22 m{1, 2,
                        3, 4};
        cgmath::Vec2 v{6, 7};
        auto result = cgmath::Mul(m, v);

        REQUIRE(result.x == 20);
        REQUIRE(result.y == 46);
    }
}