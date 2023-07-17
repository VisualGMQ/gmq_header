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

enum MyEnum {
    Value1 = 123,
    Value2 = 277,
    Value3 = 45,
};

ReflEnum(MyEnum, int) {
    Values(
        Value("Value1", MyEnum::Value1),
        Value("Value2", MyEnum::Value2),
        Value("Value3", MyEnum::Value3)
    )
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

    SECTION("enum refl") {
        static_assert(refl::EnumInfo<MyEnum>::values.size() == 3);
        static_assert(refl::EnumInfo<MyEnum>::values[0].name == "Value1");
        static_assert(refl::EnumInfo<MyEnum>::values[0].value == MyEnum::Value1);
        static_assert(refl::EnumInfo<MyEnum>::values[1].name == "Value2");
        static_assert(refl::EnumInfo<MyEnum>::values[1].value == MyEnum::Value2);
        static_assert(refl::EnumInfo<MyEnum>::values[2].name == "Value3");
        static_assert(refl::EnumInfo<MyEnum>::values[2].value == MyEnum::Value3);
    }
}
