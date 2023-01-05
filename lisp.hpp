#pragma once
#include <tuple>
#include <string>
#include <iostream>
#include <cassert>
#include <functional>

namespace lisp {

template <typename First, typename Second>
struct Cons {
    using head = First;
    using tail = Second;
};

struct Nul {};

template <int Val>
struct IntAtom {
    static constexpr decltype(Val) value = Val;
};

template <char Val>
struct CharAtom {
    static constexpr decltype(Val) value = Val;
};

template <bool Val>
struct BoolAtom {
    static constexpr decltype(Val) value = Val;
};

// functional functions
template <typename T>
struct IsAtomT {
    static constexpr bool value = false;
};

template <int Val>
struct IsAtomT<IntAtom<Val>> {
    static constexpr bool value = true;
};

template <bool Val>
struct IsAtomT<BoolAtom<Val>> {
    static constexpr bool value = true;
};

template <char Val>
struct IsAtomT<CharAtom<Val>> {
    static constexpr bool value = true;
};

template<>
struct IsAtomT<Nul> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool AtomP = IsAtomT<T>::value;

template <typename T>
constexpr bool ConsP = !AtomP<T>;

template <typename T>
constexpr bool NulP = std::is_same_v<Nul, T>;

template <typename Cons>
using Head = typename Cons::head;

template <typename Cons>
using Tail = typename Cons::tail;

template <typename T, typename... Args>
struct ListT;

template <typename T>
struct ListT<T> {
    using value_type = Cons<T, Nul>;
};

template <typename T, typename... Args>
struct ListT {
    using value_type = Cons<T, typename ListT<Args...>::value_type>;
};

template <typename... Args>
using List = typename ListT<Args...>::value_type;

template <typename T>
constexpr auto Value = T::value;

template <char C, char... Chars>
using String = List<CharAtom<C>, CharAtom<Chars>...>;

template <typename Cons>
struct LastElemT {
    using value_type = typename LastElemT<Tail<Cons>>::value_type;
};

template <int Val>
struct LastElemT<IntAtom<Val>> {
    using value_type = IntAtom<Val>;
};

template <char Val>
struct LastElemT<CharAtom<Val>> {
    using value_type = CharAtom<Val>;
};

template <bool Val>
struct LastElemT<BoolAtom<Val>> {
    using value_type = BoolAtom<Val>;
};

template <>
struct LastElemT<Nul> {
    using value_type = Nul;
};

template <typename Cons>
using LastElem = typename LastElemT<Cons>::value_type;

template <typename Cons>
static constexpr bool ListP = std::is_same_v<Nul, LastElem<Cons>>;

template <typename Elem1, typename Elem2>
static constexpr bool Equal = std::is_same_v<Elem1, Elem2>;

template <typename T>
struct DisplayT;

template <int Val>
struct DisplayT<IntAtom<Val>> {
    static constexpr int value = Val;
};

template <char Val>
struct DisplayT<CharAtom<Val>> {
    static constexpr char value = Val;
};

template <bool Val>
struct DisplayT<BoolAtom<Val>> {
    static constexpr bool value = Val;
};

template <>
struct DisplayT<Nul> {
    static constexpr const char* value = "Nul";
};

template <typename T>
auto Display = DisplayT<T>::value;

template <typename T>
void PrintList();

template <typename T>
void printListContent() {
    if constexpr (AtomP<Head<T>>) {
        std::cout << Display<Head<T>>;
        if constexpr (!Equal<Nul, Tail<T>>) {
            std::cout << " ";
        }
    } else {
        PrintList<T>();
    }
    if constexpr (!Equal<Nul, Tail<T>>) {
        printListContent<Tail<T>>();
    }
}

template <typename T>
void PrintList() {
    std::cout << '(';
    printListContent<T>();
    std::cout << ')';
}

template <typename T>
void PrintCons();

template <typename T>
void printConsContent() {
     if constexpr (ConsP<Tail<T>>) {
        if constexpr (AtomP<Head<T>>) {
            std::cout << Display<Head<T>>;
            if constexpr (!Equal<Nul, Tail<T>>) {
                std::cout << " ";
            }
        } else {
            PrintCons<Head<T>>();
        }
        printConsContent<Tail<T>>();
    } else {
        if constexpr (AtomP<Head<T>>) {
            std::cout << Display<Head<T>>;
        } else {
            PrintCons<Tail<T>>();
        }
        std::cout << " . " << Display<Tail<T>>;
    }
}

template <typename T>
void PrintCons() {
    std::cout << '(';
    printConsContent<T>();
    std::cout << ')';
}

template <typename T>
void Print() {
    if constexpr(AtomP<T>) {
        std::cout << Display<T>;
    } else if constexpr (ListP<T>){
        PrintList<T>();
    } else {
        PrintCons<T>();
    }
}

template <typename T>
void Println() {
    Print<T>();
    std::cout << std::endl;
}

}
