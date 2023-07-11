#include "refl.hpp"
#include "luabind.hpp"

struct TestClass final {
    TestClass(int a): value(a) {}

    int GetValue() const { return value; }

    void InvalidFunction() {
        std::cout << "Invalid function" << std::endl;
    }

    void PrintType(const std::string&) const {
        std::cout << "string" << std::endl;
    }
    void PrintType(float) const {
        std::cout << "number" << std::endl;
    }

    void ChangeNameFunc() {
        std::cout << "my real name is ChangeNameFunc" << std::endl;
    }

    int value;
};

ReflClass(TestClass) {
    Constructors(TestClass(int))
    Fields(
        Field("value", &TestClass::value),
        Field("GetValue", &TestClass::GetValue),
        Overload("PrintType",
                    static_cast<void(TestClass::*)(const std::string&) const>(&TestClass::PrintType),
                    static_cast<void(TestClass::*)(float) const>(&TestClass::PrintType)),
        AttrField(Attrs(luabind::LuaBindName<char, 'f', 'o', 'o'>), "ChangeNameFunc", &TestClass::ChangeNameFunc),
        AttrField(Attrs(luabind::LuaNoBind), "InvalidFunction", &TestClass::InvalidFunction),
    )
};

int main() {
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    luabind::BindClass<TestClass>(lua, "TestClass");
    lua.do_string(R"(
        local a = TestClass.new(123)
        print(a:GetValue())
        print(a.value)
        a:PrintType("haha")
        a:PrintType(123)
        if a.InvalidFunction then
            a:InvalidFunction()
        end
        if a.foo then
            a:foo()
        end
    )");
    return 0;
}