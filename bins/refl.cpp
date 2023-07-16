#include "refl.hpp"

#define CATCH_CONFIG_MAIN
#include "3rdlibs/catch.hpp"

struct TestClass final {
    float fvalue;
    double dvalue;

    static int StaticFunc(float, double) { return 123; }
    std::string MemberFunc(double&, char&&) { return ""; }

    void OverloadFunc(int) {}
    void OverloadFunc(double) {}

    TestClass(float) {}
    TestClass(double) {}
};


ReflClass(TestClass) {
    Constructors(TestClass(float), TestClass(double))
    Fields(
        Field("fvalue", &TestClass::fvalue),
        Field("dvalue", &TestClass::dvalue),
        Field("StaticFunc", TestClass::StaticFunc),
        Field("MemberFunc", &TestClass::MemberFunc),
        Overload("OverloadFunc",
                    static_cast<void(TestClass::*)(int)>(&TestClass::OverloadFunc),
                    static_cast<void(TestClass::*)(double)>(&TestClass::OverloadFunc)),
    )
};

template <typename T>
struct FiltFloat {
    static constexpr bool value = std::is_floating_point_v<T>;
};

TEST_CASE("reflection") {
    constexpr auto info = refl::TypeInfo<TestClass>();
    static_assert(std::is_same_v<decltype(info)::classType, TestClass>);

    SECTION("member1") {
        constexpr auto& member = std::get<0>(info.fields);
        using type = std::remove_const_t<std::remove_reference_t<decltype(std::get<0>(info.fields))>>;
        static_assert(member.isStatic == false);
        static_assert(std::is_same_v<type::type, float>);
        static_assert(member.name == "fvalue");
        static_assert(member.pointer == &TestClass::fvalue);
    }

    SECTION("member2") {
        constexpr auto& member = std::get<1>(info.fields);
        using type = std::remove_const_t<std::remove_reference_t<decltype(std::get<1>(info.fields))>>;
        static_assert(member.isStatic == false);
        static_assert(member.name == "dvalue");
        static_assert(std::is_same_v<type::type, double>);
        static_assert(member.pointer == &TestClass::dvalue);
    }

    SECTION("static function") {
        constexpr auto& member = std::get<2>(info.fields);
        using type = std::remove_const_t<std::remove_reference_t<decltype(std::get<2>(info.fields))>>;
        static_assert(member.isStatic == true);
        static_assert(member.name == "StaticFunc");
        static_assert(std::is_same_v<type::returnType, int>);
        static_assert(std::is_same_v<type::params, refl::ElemList<float, double>>);
        static_assert(member.pointer == &TestClass::StaticFunc);
    }

    SECTION("member function") {
        constexpr auto member = std::get<3>(info.fields);
        using type = std::remove_const_t<std::remove_reference_t<decltype(std::get<3>(info.fields))>>;
        static_assert(member.isStatic == false);
        static_assert(member.name == "MemberFunc");
        static_assert(std::is_same_v<type::returnType, std::string>);
        static_assert(std::is_same_v<type::params, refl::ElemList<double&, char&&>>);
        static_assert(member.pointer == &TestClass::MemberFunc);
        static_assert(!refl::IsOverloadFunctions<std::remove_const_t<decltype(member)>>::value);

        constexpr auto overload = std::get<4>(info.fields);
        static_assert(refl::IsOverloadFunctions<std::remove_const_t<decltype(overload)>>::value);
    }

    using Elems = refl::ElemList<double, float, int>;
    using type = refl::Filter<FiltFloat, Elems>::type;
    static_assert(std::is_same_v<type, refl::ElemList<double, float>>);
}
