#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <tuple>
#include <unordered_map>

//! @namespace refl
namespace refl {

template <typename T>
struct TypeInfoBase {
    using classType = T;
};

//! @brief function information
//! @tparam Ret return type
//! @tparam Class if function is class member function, give the class type, or give void
//! @tparam ...Params function parameters
template <typename Ret, typename Class, typename... Params>
struct FuncInfo {
    static constexpr bool isStatic = false;
    using returnType = Ret;
    using params = std::tuple<Params...>;
    using classType = Class;
    using pointerType = Ret(Class::*)(Params...);

    explicit constexpr FuncInfo(const std::string_view name, const pointerType ptr): name(name), pointer(ptr) {}

    const pointerType pointer;
    const std::string_view name;
};

// specialization for non-member function 
template <typename Ret, typename... Params>
struct FuncInfo<Ret, void, Params...> {
    static constexpr bool isStatic = true;
    using returnType = Ret;
    using params = std::tuple<Params...>;
    using classType = void;
    using pointerType = Ret(*)(Params...);

    explicit constexpr FuncInfo(const std::string_view name, const pointerType ptr): name(name), pointer(ptr) {}

    const pointerType pointer;
    const std::string_view name;
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

    explicit constexpr VariableInfo(std::string_view name, const pointerType ptr): name(name), pointer(ptr) {}

    const pointerType pointer;
    const std::string_view name;
};

// specialize for non-member variable 
template <typename Type>
struct VariableInfo<Type, void> {
    static constexpr bool isStatic = true;
    using type = Type;
    using classType = void;

    using pointerType = Type*;

    explicit constexpr VariableInfo(std::string_view name, const pointerType ptr): name(name), pointer(ptr) {}

    const pointerType pointer;
    const std::string_view name;
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

    explicit constexpr FieldInfo(const std::string_view name, const pointerType ptr): VariableInfo<T, void>(name, ptr) { }
};

// specialize for non-member function
template <typename AttrList, typename Ret, typename... Params>
struct FieldInfo<AttrList, Ret(Params...)>: public FuncInfo<Ret, void, Params...> {
    using attrs = AttrList;
    using pointerType = Ret(*)(Params...);

    explicit constexpr FieldInfo(const std::string_view name, const pointerType ptr): FuncInfo<Ret, void, Params...>(name, ptr) { }
};

// specialize for member function
template <typename AttrList, typename Ret, typename Class, typename... Params>
struct FieldInfo<AttrList, Ret(Class::*)(Params...)>: public FuncInfo<Ret, Class, Params...> {
    using attrs = AttrList;

    using pointerType = Ret(Class::*)(Params...);

    explicit constexpr FieldInfo(const std::string_view name, const pointerType ptr): FuncInfo<Ret, Class, Params...>(name, ptr) { }
};

// specialize for member variable
template <typename AttrList, typename Type, typename Class>
struct FieldInfo<AttrList, Type(Class::*)>: public VariableInfo<Type, Class> {
    using attrs = AttrList;

    using pointerType = Type Class::*;

    explicit constexpr FieldInfo(const std::string_view name, const pointerType ptr): VariableInfo<Type, Class>(name, ptr) { }
};

template <typename T>
struct TypeInfo;

// some helper macros for reflect class more easier

#define REFL_CLASS(clazz) \
template <> \
struct refl::TypeInfo<clazz>: public TypeInfoBase<clazz> { \
    static constexpr auto info = std::tuple{

#define FIELD(name, type) refl::FieldInfo<void, decltype(type)>(name, type)
#define ATTR_FIELD(attrs, name, type) refl::FieldInfo<attrs, decltype(type)>(name, type)

#define REFL_END() };};

}  // namespace refl