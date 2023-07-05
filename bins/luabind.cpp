#include "refl.hpp"
#include "luabind.hpp"

struct TestClass final {
    TestClass(int a): value(a) {}

    int GetValue() const { return value; }

    int value;
};

ReflClass(TestClass) {
    Constructors(TestClass(int))
    Fields(
        Field("value", &TestClass::value),
        Field("GetValue", &TestClass::GetValue),
    )
};

int main() {
    sol::state script = luabind::BindClass<TestClass>("TestClass");
    script.do_string(R"(
        local a = TestClass.new(123)
        print(a:GetValue())
        print(a.value)
    )");
    return 0;
}