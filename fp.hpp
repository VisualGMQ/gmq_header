#pragma once

#include <functional>
#include <memory>
#include <iostream>
#include <tuple>

//! @brief basic List data structure
//! @tparam T element type
//! @tparam ... elements
template <typename T, T...>
struct List {
    using elem_type = T;
};

//! @brief pure template implementation
namespace tmpl {

template <typename T>
struct _Head final { };

template <typename T, T first, T... Remains>
struct _Head<List<T, first, Remains...>> final {
    static constexpr auto value = first;
};

template <typename T>
struct _Tail final { };

template <typename T, T first, T... Remains>
struct _Tail<List<T, first, Remains...>> final {
    using type = List<T, Remains...>;
};

template <typename T>
struct _Last final {};

template <typename T, T first>
struct _Last<List<T, first>> final {
    static constexpr T value = first;
};

template <typename T, T first, T... Remains>
struct _Last<List<T, first, Remains...>> final {
    static constexpr T value = sizeof...(Remains) == 0 ? first : _Last<List<T, Remains...>>::value;
};

template <typename T, typename T::elem_type>
struct _Concat final {};

template <typename T, T value, T... Remains>
struct _Concat<List<T, Remains...>, value> final {
    using type = List<T, value, Remains...>;
};

template <typename ListT>
struct _Init final {};

template <typename T, T first, T last>
struct _Init <List<T, first, last>> final {
    using type = List<T, first>;
};

template <typename T, T first, T... Remains>
struct _Init<List<T, first, Remains...>> final {
    using type = typename _Concat<typename _Init<List<T, Remains...>>::type, first>::type;
};


template <typename ListT>
constexpr auto Head = _Head<ListT>::value;

template <typename ListT>
using Tail = typename _Tail<ListT>::type;

template <typename ListT>
constexpr auto Last = typename _Last<ListT>::value;

template <typename ListT1, typename ListT2>
constexpr auto Eq = std::is_same_v<ListT1, ListT2>;

template <typename T, T value, typename ListT>
using Concat = typename _Concat<ListT, value>::type;

template <typename ListT>
using Init = typename _Init<ListT>::type;

}

//! @brief pure constexpr function implementation
namespace func {

template <typename T, T first, T... Remains>
constexpr T Head(List<T, first, Remains...>) {
    return first;
}

template <typename T, T first, T... Remains>
constexpr auto Tail(List<T, first, Remains...>) {
    return List<T, Remains...>{};
}

template <typename T, T first, T... Remains>
constexpr auto Last(List<T, first, Remains...>) {
    if constexpr (sizeof...(Remains) == 0) {
        return first;
    } else {
        return Last(List<T, Remains...>{});
    }
}

template <typename T, T first, T... Remains>
constexpr auto Length(List<T, first, Remains...>) {
    return sizeof...(Remains) + 1;
}

template <typename T, T v, T... Remains>
constexpr auto Concat(std::integral_constant<T, v> value, List<T, Remains...>) {
    return List<T, value(), Remains...>{};
}

template <typename T, T first, T second>
constexpr auto Init(List<T, first, second>) {
    return List<T, first>{};
}

template <typename T, T first, T... Remains>
constexpr auto Init(List<T, first, Remains...> list) {
    using NextList = List<T, Remains...>;
    return Concat(std::integral_constant<T, first>{}, Init(NextList{}));
}

template <typename List1, typename List2>
constexpr bool Eq(List1, List2) {
    return std::is_same_v<List1, List2>;
}

}

template <typename T, T first, T... Remains>
std::ostream& operator<<(std::ostream& stream, List<T, first, Remains...>) {
    stream << first;
    if constexpr (sizeof...(Remains) != 0) {
        stream << ", ";
        return stream << List<T, Remains...>{};
    } else {
        return stream;
    }
}

