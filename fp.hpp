#pragma once

#include <functional>
#include <memory>
#include <iostream>
#include <tuple>
#include <array>
#include <string_view>

//! @brief basic List data structure
//! @tparam T element type
//! @tparam ... elements
template <typename T, T... values>
struct List {
    using elem_type = T;
    static constexpr std::array<T, sizeof...(values)> value = {values..., };
};

//! @brief a quick macro to generate a list type
#define GList(...) List<decltype(__VA_ARGS__), __VA_ARGS__>

//! @brief represent any compile-time value
template <typename T, T value>
struct TElem {
    using type = T;
    static constexpr T value = value;
};

#define GTElem(x) TElem<decltype(x), x>

//! @brief compile-time tuple
//! @tparam ...ElemsT must be Any<T, value>
template <typename... ElemsT>
struct Tuple {
    static constexpr auto value = std::tuple<ElemsT...>{};
};

//! @brief pure template implementation
namespace tmpl {

/**********************List Operations*********************/

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
struct _Tail<List<T>> final {
    using type = List<T>;
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

// concat a element to list
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

template <typename T>
struct _Init <List<T>> final {
    using type = List<T>;
};

template <typename T, T first, T... Remains>
struct _Init<List<T, first, Remains...>> final {
    using type = typename _Concat<typename _Init<List<T, Remains...>>::type, first>::type;
};

template <size_t N, typename T>
struct _Nth final {};


template <typename T, T first, T... Remains>
struct _Nth<0, List<T, first, Remains...>> final {
    static constexpr T value = first;
};

template <size_t N, typename T, T first, T... Remains>
struct _Nth<N, List<T, first, Remains...>> final {
    static constexpr T value = _Nth<N - 1, List<T, Remains...>>::value;
};

template <typename T>
struct _Length final {};

template <typename T, T... Remains>
struct _Length<List<T, Remains...>> final {
    static constexpr size_t value = sizeof...(Remains);
};

template <typename T>
struct _Null final { };

template <typename T>
struct _Null<List<T>> {
    static constexpr bool value = true;
};

template <typename T, T... Remains>
struct _Null<List<T, Remains...>> {
    static constexpr bool value = false;
};

template <typename ListT1, typename ListT2>
struct _ConcatList;


template <typename T, T value, T... Remains>
struct _ConcatList<List<T, value>, List<T, Remains...>> final {
    using type = List<T, value, Remains...>;
};

// concat two lists
template <typename ListT1, typename ListT2>
struct _ConcatList final {
    using type = typename _ConcatList<
                    typename _Init<ListT1>::type,
                    typename _Concat<ListT2, _Last<ListT1>::value>::type
                >::type;
};

template <size_t N, typename T, T value>
struct _Replicate final {
    using type = typename _Concat<typename _Replicate<N - 1, T, value>::type, value>::type;
};

template <typename T, T value>
struct _Replicate<1, T, value> final {
    using type = List<T, value>;
};

template <size_t N, typename ListT>
struct _Take;

template <typename T, T... Remains>
struct _Take<0, List<T, Remains...>> final {
    using type = List<T>;
};

template <size_t N, typename ListT>
struct _Take final {
    using type = typename _Concat<
                    typename _Take<
                        N - 1,
                        typename _Tail<ListT>::type
                    >::type,
                    _Head<ListT>::value
                >::type;
};

template <typename ListT>
struct _Sum;

template <typename T>
struct _Sum<List<T>> {
    static constexpr T value = T{};
};

template <typename T, T first, T... Remains>
struct _Sum<List<T, first, Remains...>> final {
    static constexpr T value = first + _Sum<List<T, Remains...>>::value;
};

template <typename ListT>
struct _Maxmimum;

template <typename T, T value>
struct _Maxmimum<List<T, value>> final {
    static constexpr T value = value;
};

template <typename T, T first, T... Remains>
struct _Maxmimum<List<T, first, Remains...>> final {
    static constexpr T value = std::max(first, _Maxmimum<List<T, Remains...>>::value);
};

template <typename ListT, typename ListT::elem_type elem>
struct _Elem {
    static constexpr bool value = elem == _Head<ListT>::value ? true : _Elem<typename _Tail<ListT>::type, elem>::value;
};

template <typename T, T value, T elem>
struct _Elem<List<T, value>, elem> {
    static constexpr bool value = value == elem;
};

//! @brief get first element in list
//! @warning can't compile when list is empty
//! @tparam ListT list
template <typename ListT>
constexpr auto Head = _Head<ListT>::value;

//! @brief get list without first element
//! @warning can't compile when list is empty
//! @tparam ListT list
template <typename ListT>
using Tail = typename _Tail<ListT>::type;

//! @brief get last element in list
//! @warning can't compile when list is empty
//! @tparam ListT list
template <typename ListT>
constexpr auto Last = typename _Last<ListT>::value;

//! @brief judge if two lists have same elements and order
//! @tparam ListT list
template <typename ListT1, typename ListT2>
constexpr auto Eq = std::is_same_v<ListT1, ListT2>;

//! @brief add a value ahead of list
//! @tparam ListT list
//! @tparam value the value to be added
template <typename ListT, typename ListT::elem_type value>
using Concat = typename _Concat<ListT, value>::type;

//! @brief concat two lists
template <typename List1, typename List2>
using ConcatList = typename _ConcatList<List1, List2>::type;

//! @brief get list without latest element
template <typename ListT>
using Init = typename _Init<ListT>::type;

//! @brief get Nth element in list
template <size_t N, typename ListT>
constexpr auto Nth = _Nth<N, ListT>::value;

//! @brief judge if list is empty
template <typename ListT>
constexpr bool Null = _Null<ListT>::value;

//! @brief get list length
template <typename ListT>
constexpr auto Length = _Length<ListT>::value;

template <size_t N, typename T, T value>
using Replicate = typename _Replicate<N, T, value>::type;

template <typename ListT>
constexpr auto Sum = _Sum<ListT>::value;

template <typename ListT>
constexpr auto Maxmimum= _Maxmimum<ListT>::value;

template <typename ListT, typename ListT::elem_type value>
constexpr auto Elem = _Elem<ListT, value>::value;


/**********************Tuple Operations*********************/
template <typename Tuple>
struct _Fst;

template <typename Elem1, typename Elem2>
struct _Fst<Tuple<Elem1, Elem2>> {
    static constexpr auto value = Elem1::value;
};

template <typename Tuple>
struct _Snd;

template <typename Elem1, typename Elem2>
struct _Snd<Tuple<Elem1, Elem2>> {
    static constexpr auto value = Elem2::value;
};


template <typename Tuple>
constexpr auto Fst = _Fst<Tuple>::value;

template <typename Tuple>
constexpr auto Snd = _Snd<Tuple>::value;

/**********************Other Common Operations*********************/
template <typename ListT,
          template <typename U, U value>
          typename Func
        >
struct _Map;

template <typename T, T value,
            template <typename U, U value>
            typename Func>
struct _Map<List<T, value>, Func> {
    using type = List<T, Func<T, value>::value>;
};

template <typename T, T value,
            template <typename U, U value>
            typename Func,
          T... Remains>
struct _Map<List<T, value, Remains...>, Func> {
    using type = Concat<typename _Map<List<T, Remains...>, Func>::type, Func<T, value>::value>;
};

template <typename ListT,
            template <typename T, T value>
            typename Pred>
struct _Filter;

template <typename T,
            template <typename U, U value>
            typename Pred>
struct _Filter<List<T>, Pred> {
    using type = List<T>;
};

template <typename T,
          T elem,
            template <typename U, U value>
            typename Pred,
        T... Remains>
struct _Filter<List<T, elem, Remains...>, Pred> {
    using type = std::conditional_t<
                    Pred<T, elem>::value,
                    Concat<
                        typename _Filter<List<T, Remains...>, Pred>::type,
                        elem>,
                    typename _Filter<List<T, Remains...>, Pred>::type>;
};

template <typename T, T value>
struct Inc final {
    static constexpr T value = value + 1;
};

template <typename T, T value>
struct Dec final {
    static constexpr T value = value - 1;
};

//! @brief apply function `Func` for each element
//! @tparam Func must has template declare: `template<typename T, T value>`
//! @tparam ListT applied list
template <typename ListT,
          template <typename T, T value>
          typename Func
        >
using Map = typename _Map<ListT, Func>::type;

template <typename ListT,
            template <typename U, U value>
            typename Pred>
using Filter = typename _Filter<ListT, Pred>::type;

template <typename T, T value>
struct Odd {
    static constexpr bool value = value % 2 == 1;
};

template <typename T, T value>
struct Even {
    static constexpr bool value = value % 2 == 0;
};

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

