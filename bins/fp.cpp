#include "fp.hpp"

#define CATCH_CONFIG_MAIN
#include "3rdlibs/catch.hpp"

TEST_CASE("list operations") {
    SECTION("template implementation") {
        using listT = List<int, 1, 2, 3, 4>;

        constexpr int head = tmpl::Head<listT>;
        constexpr int snd = tmpl::Head<tmpl::Tail<listT>>;
        constexpr int last = tmpl::Last<listT>;
        using tail = tmpl::Tail<listT>;
        using init = tmpl::Init<listT>;
        using concatedList = tmpl::Concat<listT::elem_type, 0, listT>;

        static_assert(tmpl::Eq<listT, List<int, 1, 2, 3, 4>>);
        static_assert(head == 1);
        static_assert(snd == 2);
        static_assert(last == 4);
        static_assert(tmpl::Eq<tail, List<int, 2, 3, 4>>);
        static_assert(tmpl::Eq<init, List<int, 1, 2, 3>>);
        static_assert(tmpl::Eq<concatedList, List<int, 0, 1, 2, 3, 4>>);


    }

    SECTION("compile-time function implementations") {
        constexpr auto list = List<int, 1, 2, 3, 4>{};

        constexpr int headF = func::Head(list);
        constexpr int sndF = func::Head(func::Tail(list));
        constexpr int lastF = func::Last(list);
        constexpr auto concatedListF = func::Concat(std::integral_constant<int, 0>{}, list);
        constexpr auto initF = func::Init(list);
        REQUIRE(func::Eq(list, List<int, 1, 2, 3, 4>{}));
        static_assert(headF == 1);
        static_assert(sndF == 2);
        static_assert(lastF == 4);
        REQUIRE(func::Eq(initF, List<int, 1, 2, 3>{}));
    }
}