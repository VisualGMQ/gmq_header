#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <tuple>
#include <unordered_map>

//! @namespace refl
namespace refl {

template <typename... Args>
struct ElemList {
    static constexpr size_t size = sizeof...(Args);
};

template <typename T>
struct TypeInfoBase {
    using classType = T;
};

//! @brief function information
//! @tparam Ret return type
//! @tparam Class if function is class member function, give the class type, or give void
//! @tparam ...Params function parameters
template <typename Ret, typename Class, typename... Params>
struct FuncInfoBase {
    static constexpr bool isStatic = false;
    using returnType = Ret;
    using params = std::tuple<Params...>;
    using classType = Class;
    using pointerType = Ret(Class::*)(Params...);

    explicit constexpr FuncInfoBase(pointerType ptr): pointer(ptr) {}

    const pointerType pointer;
};

// specialization for non-member function 
template <typename Ret, typename... Params>
struct FuncInfoBase<Ret, void, Params...> {
    static constexpr bool isStatic = true;
    using returnType = Ret;
    using params = std::tuple<Params...>;
    using classType = void;
    using pointerType = Ret(*)(Params...);

    explicit constexpr FuncInfoBase(pointerType ptr): pointer(ptr) {}

    const pointerType pointer;
};

template <typename Ret, typename Class, typename... Params>
struct FuncInfo: public FuncInfoBase<Ret, Class, Params...> {
    static constexpr bool hasOverload = false;

    constexpr FuncInfo(std::string_view name,
                       typename FuncInfoBase<Ret, Class, Params...>::pointerType ptr)
        : FuncInfoBase<Ret, Class, Params...>(ptr), name(name) {}

    std::string_view name;
};

//! @brief variable information
//! @tparam Type variable type
//! @tparam Class if variable is class member, give the class type, or give void
template <typename Type, typename Class>
struct VariableInfo {
    static constexpr bool isStatic = false;
    using type = Type;
    using classType = Class;

    using pointerType = Type Class::*;

    explicit constexpr VariableInfo(std::string_view name, pointerType ptr): name(name), pointer(ptr) {}

    const pointerType pointer;
    std::string_view name;
};

// specialize for non-member variable 
template <typename Type>
struct VariableInfo<Type, void> {
    static constexpr bool isStatic = true;
    using type = Type;
    using classType = void;

    using pointerType = Type*;

    explicit constexpr VariableInfo(std::string_view name, pointerType ptr): name(name), pointer(ptr) {}

    const pointerType pointer;
    std::string_view name;
};


template <typename... Attrs>
struct AttrList {};

//! @brief field information(member/non-member function, variable)
//! @tparam Attr some custom attribute binding on this field
//! @tparam T 
template <typename AttrList, typename T>
struct FieldInfo: public VariableInfo<T, void> {
    using attrs = AttrList;
    using pointerType = T*;

    explicit constexpr FieldInfo(std::string_view name, pointerType ptr): VariableInfo<T, void>(name, ptr) { }
};

// specialize for non-member function
template <typename AttrList, typename Ret, typename... Params>
struct FieldInfo<AttrList, Ret(Params...)>: public FuncInfo<Ret, void, Params...> {
    using attrs = AttrList;
    using pointerType = Ret(*)(Params...);

    explicit constexpr FieldInfo(std::string_view name, pointerType ptr): FuncInfo<Ret, void, Params...>(name, ptr) { }
};

// specialize for member function
template <typename AttrList, typename Ret, typename Class, typename... Params>
struct FieldInfo<AttrList, Ret(Class::*)(Params...)>: public FuncInfo<Ret, Class, Params...> {
    using attrs = AttrList;
    using pointerType = Ret(Class::*)(Params...);

    explicit constexpr FieldInfo(std::string_view name, pointerType ptr): FuncInfo<Ret, Class, Params...>(name, ptr) { }
};

// specialize for member variable
template <typename AttrList, typename Type, typename Class>
struct FieldInfo<AttrList, Type(Class::*)>: public VariableInfo<Type, Class> {
    using attrs = AttrList;
    using pointerType = Type Class::*;

    constexpr FieldInfo(std::string_view name, pointerType ptr): VariableInfo<Type, Class>(name, ptr) { }
};

template <typename T>
struct TypeInfo;

// some helper macros for reflect class more easier

#define ReflClass(clazz) \
template <> \
struct refl::TypeInfo<clazz>: public TypeInfoBase<clazz>

#define Fields(...) static constexpr auto fields = std::tuple{ __VA_ARGS__ };
#define Constructors(...) using constructors = ElemList<__VA_ARGS__>;

#define Field(name, type) refl::FieldInfo<void, decltype(type)>(name, type)
#define AttrField(attrs, name, type) refl::FieldInfo<attrs, decltype(type)>(name, type)

// some other help functions:

template <size_t Idx, typename... Args>
constexpr int _countOverloadFunctionNum(std::string_view name, std::tuple<Args...> fields) {
    if constexpr (Idx >= sizeof...(Args)) {
        return 0;
    } else {
        return (std::get<Idx>(fields).name == name ? 1 : 0) + _countOverloadFunctionNum<Idx + 1>(name, fields);
    }
}

//! @brief Judge whether a class function has overloadd
//! @tparam Class class you reflected
//! @param name  function name
//! @return 
template <typename Class>
constexpr bool HasOverloadFunction(std::string_view name) {
    return _countOverloadFunctionNum<0>(name, refl::TypeInfo<Class>::fields) > 1;
}

}  // namespace refl
