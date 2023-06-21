#include "fp.hpp"

#define CATCH_CONFIG_MAIN
#include "3rdlibs/catch.hpp"

using l = GList(1, 2, 3, 4);

TEST_CASE("list operations") {
    SECTION("template implementation") {
        using listT = GList(1, 2, 3, 4);

        constexpr int head = tmpl::Head<listT>;
        constexpr int snd = tmpl::Head<tmpl::Tail<listT>>;
        constexpr int last = tmpl::Last<listT>;
        using tail = tmpl::Tail<listT>;
        using init = tmpl::Init<listT>;
        using concatedList = tmpl::Concat<listT, 0>;

        static_assert(tmpl::Eq<listT, List<int, 1, 2, 3, 4>>);
        static_assert(head == 1);
        static_assert(snd == 2);
        static_assert(last == 4);
        static_assert(tmpl::Eq<tail, GList(2, 3, 4)>);
        static_assert(tmpl::Eq<init, GList(1, 2, 3)>);
        static_assert(tmpl::Eq<concatedList, GList(0, 1, 2, 3, 4)>);
        static_assert(tmpl::Nth<0, listT> == 1);
        static_assert(tmpl::Nth<1, listT> == 2);
        static_assert(tmpl::Nth<2, listT> == 3);
        static_assert(tmpl::Nth<3, listT> == 4);
        static_assert(tmpl::Null<List<int>>);
        static_assert(tmpl::Eq<tmpl::Replicate<3, int, 1>, GList(1, 1, 1)>);
        static_assert(tmpl::Eq<tmpl::ConcatList<GList(1, 2, 3), GList(4, 5, 6)>, GList(1, 2, 3, 4, 5, 6)>);
        static_assert(tmpl::Eq<tmpl::_Take<3, GList(1, 2, 3, 4)>::type, GList(1, 2, 3)>);
        static_assert(tmpl::Sum<GList(1, 2, 3)> == 6);
        static_assert(tmpl::Maxmimum<GList(-2, 5, 9, 1, 3)> == 9);
        static_assert(tmpl::Elem<GList(-2, 5, 9, 1, 3), -2>);
        static_assert(tmpl::Elem<GList(-2, 5, 9, 1, 3), 5>);
        static_assert(!tmpl::Elem<GList(-2, 5, 9, 1, 3), 0>);
        static_assert(tmpl::Fst<Tuple<GTElem(1), GTElem(2)>> == 1);
        static_assert(tmpl::Snd<Tuple<GTElem(1), GTElem(2)>> == 2);
        static_assert(tmpl::Eq<tmpl::Map<GList(1, 2, 3, 4), tmpl::Inc>, GList(2, 3, 4, 5)>);
        static_assert(tmpl::Eq<tmpl::Filter<GList(1, 2, 3, 4), tmpl::Odd>, GList(1, 3)>);
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