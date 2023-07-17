#pragma once
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "refl.hpp"
#include <utility>

namespace luabind {

//! @brief an attribute struct hint to don't bind to lua
struct LuaNoBind final {};

//! @brief an attribute struct hint to bind field to a point name
template <typename C, C... chars>
struct LuaBindName final {
    struct Helper {
        static constexpr char name[] = {chars..., 0};
    };

    static constexpr std::string_view name = Helper::name;
};

template <typename T>
void BindClass(sol::state& lua, std::string_view name) {
    sol::usertype<T> usertype;
    using ClassInfo = refl::TypeInfo<T>;
    if constexpr(ClassInfo::constructors::size != 0) {
        ClassInfo classinfo;
        usertype = std::move(_CtorBindHelper<T, typename refl::Filter<_HasNoRValueRefParamInFunc, ClassInfo::constructors>::type>::BindAndCreate(name, lua));
    }
    _bindFields<ClassInfo>(usertype);
}


template <typename T, typename CtorList>
struct _CtorBindHelper;

//! @brief a help struct to detect whether parameters is rvalue-reference
template <typename ParamList>
struct _HasRValueRefParam;

template <typename T, typename... Args>
struct _HasRValueRefParam<refl::ElemList<T, Args...>> {
    static constexpr bool value = std::is_rvalue_reference_v<T> || _HasRValueRefParam<refl::ElemList<Args...>>::value;
};

template <>
struct _HasRValueRefParam<refl::ElemList<>> {
    static constexpr bool value = false;
};

template <typename F>
struct _HasNoRValueRefParamInFunc {
    static constexpr bool value = !_HasRValueRefParam<refl::FuncInfoBase<F>::params>::value;
};

template <typename T, typename... Types>
constexpr auto TuplePushfront(T value, std::tuple<Types...> tpl) {
    return std::tuple_cat(std::make_tuple(value), tpl);
}

template <typename... Types>
constexpr auto FirstOfTuple(std::tuple<Types...> t) {
    return std::get<0>(t);
}

template <size_t idx, typename... Types>
constexpr auto FilterNoRValueRefFunc(std::tuple<Types...> t) {
    if constexpr (idx == sizeof...(Types)) {
        return std::make_tuple();
    } else {
        if constexpr (_HasRValueRefParam<
                          refl::FuncInfoBase<std::tuple_element_t<
                              idx, std::tuple<Types...>>>::params>::value) {
            return FilterNoRValueRefFunc<idx + 1>(t);
        } else {
            return TuplePushfront(std::get<idx>(t), FilterNoRValueRefFunc<idx + 1>(t));
        }
    }
}

enum _AttrType: uint8_t {
    Bind = 0,
    ChangeName,
    NoBind,
};

template <typename T>
struct _GetAttrType {
    static constexpr uint8_t value = _AttrType::Bind;
};

template <>
struct _GetAttrType<LuaNoBind> {
    static constexpr _AttrType value = _AttrType::NoBind;
};

template <typename C, C... chars>
struct _GetAttrType<LuaBindName<C, chars...>> {
    static constexpr _AttrType value = _AttrType::ChangeName;
};


template <typename AttrList>
struct _GetHighLevelAttr;

template <typename T, typename... Attrs>
struct _GetHighLevelAttr<refl::AttrList<T, Attrs...>> {
    static constexpr uint8_t _value1 = _GetAttrType<T>::value;
    using _later_attr_info = _GetHighLevelAttr<Attrs...>;
    static constexpr uint8_t _value2 = _later_attr_info::value;

    static constexpr _AttrType value = static_cast<_AttrType>(_value1 > _value2 ? _value1 : _value2);
    using type = std::conditional_t<(_value1 > _value2), T, typename _later_attr_info::type>;
};

template <typename T>
struct _GetHighLevelAttr<refl::AttrList<T>> {
    static constexpr uint8_t value = _GetAttrType<T>::value;
    using type = T;
};

template <typename T, typename... Ctors>
struct _CtorBindHelper<T, refl::ElemList<Ctors...>> {
    static auto BindAndCreate(std::string_view name, sol::state& lua){
        return lua.new_usertype<T>(name, sol::constructors<Ctors...>());
    }
};

template <typename ClassInfo>
void _bindFields(sol::usertype<typename ClassInfo::classType>& usertype) {
    if constexpr (std::tuple_size_v<decltype(ClassInfo::fields)>) {
        _bindOneField<typename ClassInfo::classType, 0>(usertype, ClassInfo::fields);
    }
}

template <typename T, typename... Funcs>
void _bindOverloadFuncs(sol::usertype<T>& usertype, const std::tuple<Funcs...>& funcs, std::string_view name) {
    if constexpr (std::tuple_size_v<std::tuple<Funcs...>> > 0) {
        usertype[name] = sol::overload(std::get<Funcs>(funcs)...);
    }
}

// @brief a helper function to convert C++ operator overload to lua method
inline std::optional<std::string_view> _getOperatorLuaName(std::string_view name, bool hasParams) {
    if (name == "operator+") {
        return "__add";
    }
    if (name == "operator-" && hasParams) {
        return "__sub";
    }
    if (name == "operator-" && !hasParams) {
        return "__unm";
    }
    if (name == "operator*") {
        return "__mul";
    }
    if (name == "operator/") {
        return "__div";
    }
    if (name == "operator<") {
        return "__lt";
    }
    if (name == "operator==") {
        return "__eq";
    }
    if (name == "operator()") {
        return "__call";
    }
    return std::nullopt;
}

template <typename T, size_t Idx, typename... Fields>
void _bindOneField(sol::usertype<T>& usertype, const std::tuple<Fields...>& fields) {
    auto field = std::get<Idx>(fields);
    using type = std::tuple_element_t<Idx, std::tuple<Fields...>>;
    using high_attr = _GetHighLevelAttr<type::attrs>;
    constexpr auto attr = high_attr::value;
    if constexpr (attr != _AttrType::NoBind) {
        // detect convertable name
        std::string_view name = field.name;
        if constexpr (!refl::IsOverloadFunctions<type>::value) {
            if constexpr (std::is_member_function_pointer_v<typename type::pointerType>) {
                using params = typename type::params;
                if constexpr (!_HasRValueRefParam<params>::value) {
                    auto operatorName = _getOperatorLuaName(name, params::size != 0);
                    if (operatorName.has_value()) {
                        name = operatorName.value();
                    }
                }
            }
        }

        if constexpr (refl::IsOverloadFunctions<type>::value) {
            _bindOverloadFuncs<T>(usertype, FilterNoRValueRefFunc<0>(field.funcs), name);
        } else {
            if constexpr (attr == _AttrType::ChangeName) {
                usertype[high_attr::type::name] = field.pointer;
            } else {
                if constexpr (std::is_member_function_pointer_v<typename type::pointerType>) {
                    if constexpr (!_HasRValueRefParam<typename type::params>::value) {
                        usertype[name] = field.pointer;
                    }
                } else {
                    usertype[name] = field.pointer;
                }
            }
        }
    }

    if constexpr (Idx + 1 < sizeof...(Fields)) {
        _bindOneField<T, Idx + 1>(usertype, fields);
    }   
}


template <typename T>
void BindEnum(sol::state& lua, std::string_view name) {
    using type = refl::EnumInfo<T>;
    constexpr auto values = type::values;
    _bindEnum(lua, name, values, std::make_index_sequence<values.size()>());
}

template <typename T, size_t... Idx>
void _bindEnum(sol::state& lua, std::string_view name, T values, std::index_sequence<Idx...>) {
    lua.new_enum(name, {std::pair<std::string_view, int>(values[Idx].name, values[Idx].value)...});
}

}