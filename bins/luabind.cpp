#include "refl.hpp"
#include "luabind.hpp"

struct Person {
    Person(const std::string& name): name(name) {}
    std::string name;
};

struct TestClass final {
    TestClass(int a): value(a) {}
    TestClass(Person&& p): person(std::move(p)) {}

    int GetValue() const { return value; }

    void SetValue(int v) {
        value = std::move(v);
    }

    void SetPerson(Person&& p) {
        person = std::move(p);
    }

    void InvalidFunction() {
        std::cout << "Invalid function" << std::endl;
    }

    void PrintType(const std::string&) const {
        std::cout << "string" << std::endl;
    }
    void PrintType(float) const {
        std::cout << "number" << std::endl;
    }
    void PrintType(Person&& p) const {
        std::cout << "person" << std::endl;
    }

    void ChangeNameFunc() {
        std::cout << "my real name is ChangeNameFunc" << std::endl;
    }

    TestClass operator+(float value) const {
        std::cout << "plused " << value << std::endl;
        return *this;
    }

    int value;

    Person person{"no-name"};
};

ReflClass(Person) {
    Constructors(Person(const std::string&))
    Fields()
};

ReflClass(TestClass) {
    Constructors(TestClass(int), TestClass(Person&&))
    Fields(
        Field("value", &TestClass::value),
        Field("GetValue", &TestClass::GetValue),
        Field("SetValue", &TestClass::SetValue),
        Field("SetPerson", &TestClass::SetPerson),
        Field("operator+", &TestClass::operator+),
        Overload("PrintType",
                    static_cast<void(TestClass::*)(const std::string&) const>(&TestClass::PrintType),
                    static_cast<void(TestClass::*)(Person&&) const>(&TestClass::PrintType),
                    static_cast<void(TestClass::*)(float) const>(&TestClass::PrintType)),
        AttrField(Attrs(luabind::LuaBindName<char, 'f', 'o', 'o'>), "ChangeNameFunc", &TestClass::ChangeNameFunc),
        AttrField(Attrs(luabind::LuaNoBind), "InvalidFunction", &TestClass::InvalidFunction),
    )
};


struct Name {
    Name(Name&&) = default;
    void SetValue(int&& value) { this->value = value; }
    int value;
};

ReflClass(Name) {
    Constructors(Name(Name&&))
    Fields(
        Overload("SetValue", static_cast<void(Name::*)(int&&)>(&Name::SetValue))
    )
};

int main() {
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    luabind::BindClass<TestClass>(lua, "TestClass");
    luabind::BindClass<Name>(lua, "Name");
    lua.script(R"(
        function Test()
            local a = TestClass.new(123)
            print(a:GetValue())
            print(a.value)
            a = a + 5.0
            a:SetValue(234)
            print(a.value)
            a:PrintType("haha")
            a:PrintType(123)
            if a.InvalidFunction then
                a:InvalidFunction()
            end
            if a.foo then
                a:foo()
            end
        end
    )");

	sol::protected_function f(lua["Test"]);
    sol::protected_function_result result = f();
	if (!result.valid()) {
		sol::error err = result;
		std::string what = err.what();
		std::cout << "call failed, sol::error::what() is:" << std::endl << what << std::endl;
	}

    return 0;
}