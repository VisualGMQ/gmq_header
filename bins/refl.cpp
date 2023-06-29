#include "refl.hpp"

#define CATCH_CONFIG_MAIN
#include "3rdlibs/catch.hpp"

struct TestClass final {
    float fvalue;
    double dvalue;

    static int StaticFunc(float, double) { return 123; }
    std::string MemberFunc(double&, char&&) { return ""; }
};

REFL_CLASS(TestClass)
    FIELD("fvalue", &TestClass::fvalue),
    FIELD("dvalue", &TestClass::dvalue),
    FIELD("StaticFunc", TestClass::StaticFunc),
    FIELD("MemberFunc", &TestClass::MemberFunc),
REFL_END()

TEST_CASE("reflection") {
    constexpr auto info = refl::TypeInfo<TestClass>();
    static_assert(std::is_same_v<decltype(info)::classType, TestClass>);

    SECTION("member1") {
        constexpr auto& member = std::get<0>(info.info);
        using type = std::remove_const_t<std::remove_reference_t<decltype(std::get<0>(info.info))>>;
        static_assert(member.isStatic == false);
        static_assert(std::is_same_v<type::type, float>);
        static_assert(member.name == "fvalue");
        static_assert(member.pointer == &TestClass::fvalue);
    }

    SECTION("member2") {
        constexpr auto& member = std::get<1>(info.info);
        using type = std::remove_const_t<std::remove_reference_t<decltype(std::get<1>(info.info))>>;
        static_assert(member.isStatic == false);
        static_assert(member.name == "dvalue");
        static_assert(std::is_same_v<type::type, double>);
        static_assert(member.pointer == &TestClass::dvalue);
    }

    SECTION("static function") {
        constexpr auto& member = std::get<2>(info.info);
        using type = std::remove_const_t<std::remove_reference_t<decltype(std::get<2>(info.info))>>;
        static_assert(member.isStatic == true);
        static_assert(member.name == "StaticFunc");
        static_assert(std::is_same_v<type::returnType, int>);
        static_assert(std::is_same_v<type::params, std::tuple<float, double>>);
        static_assert(member.pointer == &TestClass::StaticFunc);
    }

    SECTION("member function") {
        constexpr auto member = std::get<3>(info.info);
        using type = std::remove_const_t<std::remove_reference_t<decltype(std::get<3>(info.info))>>;
        static_assert(member.isStatic == false);
        static_assert(member.name == "MemberFunc");
        static_assert(std::is_same_v<type::returnType, std::string>);
        static_assert(std::is_same_v<type::params, std::tuple<double&, char&&>>);
        static_assert(member.pointer == &TestClass::MemberFunc);
    }
}
