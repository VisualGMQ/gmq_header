#include "tweeny.hpp"

#define CATCH_CONFIG_MAIN
#include "3rdlibs/catch.hpp"

TEST_CASE("tweeny", "[tweeny]") {
    tweeny::Tween<float> tween = tweeny::Tween<float>::From(0).To(100).During(4)
                                                        .To(300).During(2);
    REQUIRE(tween.Direction() == tweeny::TweenyDirection::Forward);
    REQUIRE(tween.CurTick() == 0.0);
    REQUIRE(tween.CurValue() == 0.0);
    tween.Step(1);
    REQUIRE(tween.CurTick() == 1.0);
    REQUIRE(tween.CurValue() == 25.0);
    tween.Step(1);
    REQUIRE(tween.CurTick() == 2.0);
    REQUIRE(tween.CurValue() == 50.0);
    tween.Step(1);
    REQUIRE(tween.CurTick() == 3.0);
    REQUIRE(tween.CurValue() == 75.0);
    tween.Step(1);
    REQUIRE(tween.CurTick() == 4.0);
    REQUIRE(tween.CurValue() == 100.0);
    tween.Step(1);
    REQUIRE(tween.CurTick() == 5.0);
    REQUIRE(tween.CurValue() == 200.0);
    tween.Step(1);
    REQUIRE(tween.CurTick() == 6.0);
    REQUIRE(tween.CurValue() == 300.0);
    tween.Step(1);
    REQUIRE(tween.CurTick() == 6.0);
    REQUIRE(tween.CurValue() == 300.0);
}


