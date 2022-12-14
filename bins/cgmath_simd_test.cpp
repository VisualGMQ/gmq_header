#define USE_SIMD
#include "cgmath.hpp"

#define CATCH_CONFIG_MAIN
#include "3rdlibs/catch.hpp"

#include <limits>

#define FLT_EQ(a, b) (std::abs(a - b) <= std::numeric_limits<float>::epsilon())

#define USE_SIMD
TEST_CASE("simd vector4 algorithms", "[vector]") {
    cgmath::Vec4 v1{1, 2, 3, 4};
    cgmath::Vec4 v2{5, 6, 7, 8};

    SECTION("get x, y, z, w") {
        REQUIRE(v1.x() == 1);
        REQUIRE(v1.y() == 2);
        REQUIRE(v1.z() == 3);
        REQUIRE(v1.w() == 4);
        REQUIRE(v2.x() == 5);
        REQUIRE(v2.y() == 6);
        REQUIRE(v2.z() == 7);
        REQUIRE(v2.w() == 8);
    }

    SECTION("add") {
        auto result = v1 + v2;
        REQUIRE(result == cgmath::Vec4{6, 8, 10, 12});
    }

    SECTION("sub") {
        auto result = v1 - v2;
        REQUIRE(result == cgmath::Vec4{-4, -4, -4, -4});
    }
  
    SECTION("mul") {
        auto result = v1 * v2;
        REQUIRE(result == cgmath::Vec4{5, 12, 21, 32});
    }

    SECTION("div") {
        auto result = v1 / v2;
        REQUIRE(result == cgmath::Vec4{0.2, 1.0/3.0, 3.0/7.0, 0.5});
    }
  
    SECTION("dot") {
        auto result = v1.Dot(v2);
        REQUIRE(result == 70);
    }     
}
