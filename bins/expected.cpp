#include "expected.hpp"

#define CATCH_CONFIG_MAIN
#include "3rdlibs/catch.hpp"

#include <iostream>

gmq::expected<void, int> doSomethingFailed() {
    return gmq::unexpected<int>(-1);
}

gmq::expected<void, int> doSomethingOk() {
    return gmq::expected<void, int>();
}


TEST_CASE("expected test", "[expected]") {
    gmq::unexpected<int> e(123);
    REQUIRE(e.error() == 123);

    auto expected = doSomethingFailed();
    REQUIRE_FALSE(expected.has_value());
    REQUIRE(expected.error() == -1);

    expected = doSomethingOk();
    REQUIRE(expected.has_value());

    gmq::expected<int, float> e2(123);
    REQUIRE(e2);
    REQUIRE(e2.value() == 123);

    e2 = gmq::unexpected<float>(456.0f);
    REQUIRE(!e2);
    REQUIRE(e2.error() == 456.0f);
}
