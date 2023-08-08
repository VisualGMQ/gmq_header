#define CATCH_CONFIG_MAIN
#include "3rdlibs/catch.hpp"

#include "signal.hpp"

int Foo(float value) {
    return value;
}

struct IncOrDec {
    int Inc(float value) {
        return value + v;
    }

    int Dec(float value) const {
        return value - v;
    }

    float v = 0.0;
};

TEST_CASE("delegate") {
    SECTION("simple function delegate") {
        signal::Delegate<int(float)> delegate;
        delegate.Connect<&Foo>();
        REQUIRE(delegate(123.23) == 123);
    }

    SECTION("simple function delegate with payload") {
        signal::Delegate<int()> delegate;
        float value = 123.23;
        delegate.Connect<&Foo>(value);
        REQUIRE(delegate() == 123);
    }

    SECTION("member function delegate") {
        IncOrDec iod;
        iod.v = 1;
        signal::Delegate<int(float)> delegate;
        delegate.Connect<&IncOrDec::Inc>(iod);
        REQUIRE(delegate(1.23) == 2);

        signal::Delegate<int(IncOrDec&, float)> delegate2;
        delegate2.Connect<&IncOrDec::Inc>();
        REQUIRE(delegate2(iod, 1.23) == 2);
    }

    SECTION("const member function delegate") {
        IncOrDec iod;
        iod.v = 1;
        signal::Delegate<int(float)> delegate;
        delegate.Connect<&IncOrDec::Dec>(iod);

        REQUIRE(delegate(1.23) == 0);
    }

    SECTION("member variable delegate") {
        IncOrDec iod;
        iod.v = 1;
        signal::Delegate<float(void)> delegate;
        delegate.Connect<&IncOrDec::v>(iod);

        REQUIRE(delegate() == 1);
    }
}
