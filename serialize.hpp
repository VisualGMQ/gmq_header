
#include "refl.hpp"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "log.hpp"

namespace serialize {

struct DeserialPath {
    std::vector<std::string> path;

    std::string ToString() const {
        std::string str;
        for (const auto& p : path) {
            str += "->" + p;
        }
        return str;
    }
};

//! @brief serialize class to lua
//! @tparam T  the class you want to serialize
//! @return the serialized lua code, please write then into lua file
template <typename T>
std::vector<uint8_t> SerializeToLua() {
    // TODO: not finish
    return {};
}

//! @brief descrialize class from lua file
//! @tparam T  the class you want to deserialize
//! @param lua 
//! @return the class instance, failed will return std::nullopt
template <typename T>
std::optional<T> DeserializeFromLua(sol::object& lua) {
    DeserialPath path;
    return _deserializeFromLua<T>(lua, path);
}

template <typename T>
std::optional<T> _deserializeFromLua(sol::object& lua, DeserialPath& path) {
    if constexpr (std::is_class_v<T> && !_isSerialDirectly<T>()) {
        using typeinfo = refl::TypeInfo<T>;
        if (lua.get_type() == sol::type::table) {
            T value;
            _deserializeField<0>(value, lua.as<sol::table>(), typeinfo::fields, path);
            return value;
        } else {
            LOGW("[Lua Config Parser]: try to parse a non-table lua node to a class. Path: ", path.ToString());
            return std::nullopt;
        }
    } else {
        if constexpr (_isVector<T>::value) {
            using inner_type = _isVector<T>::value_type;
            std::vector<inner_type> values;
            sol::table table = lua.as<sol::table>();
            for (auto& elem : table) {
                auto elem_value = _deserializeFromLua<inner_type>(elem.second, path);
                if (elem_value.has_value()) {
                    values.push_back(elem_value.value());
                } else {
                    LOGW("[Lua Config Parser]: parse std::vector<T> failed. Path: ", path.ToString());
                    return std::nullopt;
                }
            }
            return values;
        } else if constexpr (_isArray<T>::value) {
            using inner_type = _isArray<T>::value_type;
            constexpr size_t require_size = _isArray<T>::size;
            sol::table table = lua.as<sol::table>();
            if (table.size() <require_size) {
                LOGW("[Lua Config Parser]: parse std::array failed, require ", require_size, " elements but has ", table.size(), ". Path: ", path.ToString());
                return std::nullopt;
            } else {
                std::array<inner_type, require_size> datas;
                int i = 0;
                for (auto& elem : table) {
                    auto elem_value = _deserializeFromLua<inner_type>(elem.second, path);
                    if (elem_value.has_value()) {
                        datas[i++] = elem_value.value();
                    } else {
                        LOGW("[Lua Config Parser]: parse std::array element failed. Path: ", path.ToString());
                        return std::nullopt;
                    }
                }
                return datas;
            }
        } else if constexpr (_isUnorderedMap<T>::value) {
            using key = _isUnorderedMap<T>::key;
            using type = _isUnorderedMap<T>::type;
            std::unordered_map<key, type> result;
            sol::table table = lua.as<sol::table>();
            for (auto& elem : table) {
                auto first = _deserializeFromLua<key>(elem.first, path);
                if (!first) {
                    LOGW("[Lua Config Parser]: parse std::unordered_map key element failed. Path: ", path.ToString());
                    return std::nullopt;
                }
                auto second = _deserializeFromLua<type>(elem.second, path);
                if (!second) {
                    LOGW("[Lua Config Parser]: parse std::unordered_map value element failed. Path: ", path.ToString());
                    return std::nullopt;
                }
                result[first.value()] = second.value();
            }
            return result;
        } else {
            return lua.as<T>();
        }
    }
}

inline bool _canSerialDirectly(const sol::object& lua) {
    auto type = lua.get_type();
    return type == sol::type::boolean ||
           type == sol::type::number ||
           type == sol::type::string ||
           type == sol::type::function ||
           type == sol::type::nil ||
           type == sol::type::lua_nil;
}

template <typename T>
struct _isVector {
    static constexpr bool value = false;
};

template <typename T>
struct _isVector<std::vector<T>> {
    static constexpr bool value = true;
    using value_type = T;
};

template <typename T>
struct _isArray {
    static constexpr bool value = false;
};

template <typename T, size_t N>
struct _isArray<std::array<T, N>> {
    static constexpr bool value = true;
    using value_type = T;
    static constexpr size_t size = N;
};

template <typename T>
struct _isUnorderedMap {
    static constexpr bool value = false;
};

template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
struct _isUnorderedMap<std::unordered_map<Key, T, Hash, KeyEqual, Allocator>> {
    static constexpr bool value = true;
    using key = Key;
    using type = T;
};


template <typename T>
constexpr bool _isSerialDirectly() {
    return std::is_arithmetic_v<T> ||
           std::is_same_v<T, std::string> ||
           _isVector<T>::value ||
           _isUnorderedMap<T>::value ||
           _isArray<T>::value;
}

template <size_t Idx, typename T, typename... Types>
void _deserializeField(T& value, sol::table& lua, std::tuple<Types...> fields, DeserialPath& path) {
    auto field = std::get<Idx>(fields);
    using field_info = std::tuple_element_t<Idx, std::tuple<Types...>>;
    path.path.push_back(std::string(field.name.data()));
    auto lua_field = lua[field.name];
    if (lua_field.valid()) {
        if constexpr (_isSerialDirectly<field_info::type>()) {
            auto field_value = _deserializeFromLua<field_info::type>(lua_field.get<sol::object>(), path);
            if (field_value) {
                value.*field.pointer = field_value.value();
            } else {
                LOGW("[Lua Config Parser]: parsing failed, no exists field. Path: ", path.ToString());
                return;
            }
        } else {
            if (lua_field.get_type() == sol::type::table) {
                auto field_value = _deserializeFromLua<field_info::type>(lua_field.get<sol::table>(), path);
                if (field_value) {
                    value.*field.pointer = field_value.value();
                } else {
                    LOGW("[Lua Config Parser]: parsing failed, no exists field. Path: ", path.ToString());
                    return;
                }
            }
        }
    }

    if constexpr (Idx < sizeof...(Types) - 1) {
        _deserializeField<Idx + 1>(value, lua, fields, path);
    }
}

}