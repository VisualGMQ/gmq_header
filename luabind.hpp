#pragma once
#include "sol/sol.hpp"
#include "refl.hpp"

namespace luabind {

template <typename T>
sol::state BindClass(std::string_view name) {
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    sol::usertype<T> usertype;
    using ClassInfo = refl::TypeInfo<T>;
    if constexpr(ClassInfo::constructors::size != 0) {
        ClassInfo classinfo;
        usertype = std::move(CtorBindHelper<T, ClassInfo::constructors>::BindAndCreate(name, lua));
    }
    _bindFields<ClassInfo>(usertype);
    return lua;
}

template <typename T, typename CtorList>
struct CtorBindHelper;

template <typename T, typename... Ctors>
struct CtorBindHelper<T, refl::ElemList<Ctors...>> {
    static auto BindAndCreate(std::string_view name, sol::state& lua){
        return lua.new_usertype<T>(name, sol::constructors<Ctors...>());
    }
};

template <typename ClassInfo>
void _bindFields(sol::usertype<typename ClassInfo::classType>& usertype) {
    _bindOneField<ClassInfo::classType, 0>(usertype, ClassInfo::fields);
}

template <typename T, size_t Idx, typename... Fields>
void _bindOneField(sol::usertype<T>& usertype, const std::tuple<Fields...>& fields) {
    auto field = std::get<Idx>(fields);
    if (!refl::HasOverloadFunction<T>(field.name)) {
        usertype[field.name] = field.pointer;
        if constexpr (Idx + 1 < sizeof...(Fields)) {
            _bindOneField<T, Idx + 1>(usertype, fields);
        }
    } else {
        // TODO: overload function bind
    }
}

}