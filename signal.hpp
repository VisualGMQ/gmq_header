#pragma once

#include <type_traits>

namespace signal {

namespace internal {

//! @brief extract class type from function type
//! @tparam Func 
template <typename Func>
struct class_function_traits;

template <typename Class, typename Ret, typename... Args>
struct class_function_traits<Ret(Class::*)(Args...)> {
    using type = Class;
};

template <typename Class, typename Ret, typename... Args>
struct class_function_traits<Ret(Class::*)(Args...)const> {
    using type = Class;
};

template <typename Class, typename Ret>
struct class_function_traits<Ret(Class::*)> {
    using type = Class;
};

template <typename Func>
using class_function_traits_v = typename class_function_traits<Func>::type;


//! @brief contains a bunch of types
//! @tparam ...Args 
template <typename... Args>
struct type_list {};

//! @brief provide a convinient way to index element in type_list
//! @tparam ...Args 
//! @tparam Index 
template <size_t Index, typename List>
struct type_list_elements;

template <size_t Index, typename First, typename... Args>
struct type_list_elements<Index, type_list<First, Args...>>: type_list_elements<Index - 1u, type_list<Args...>> {};

template <typename First, typename... Args>
struct type_list_elements<0u, type_list<First, Args...>> {
    using type = First;
};

template <size_t Index, typename List>
using type_list_elements_t = typename type_list_elements<Index, List>::type;

//! @brief transcribe the constness from From to To
//! @tparam To 
//! @tparam From 
template <typename To, typename From>
struct constness_as {
    using type = std::remove_const_t<To>;
};

template <typename To, typename From>
struct constness_as<To, const From> {
    using type = const To;
};

template <typename To, typename From>
using constness_as_t = typename constness_as<To, From>::type;

}

template <typename Func>
class Delegate;

template <typename Ret, typename... Args>
class Delegate<Ret(Args...)> final {
public:
    using DelegateType = Ret(*)(const void*, Args...);

    template <auto Func>
    void Connect() noexcept {
        fn_ = wrap<Func>(std::make_index_sequence<sizeof...(Args)>{});
    }

    void Connect(DelegateType d) {
        payload_ = nullptr;
        fn_ = d;
    }

    template <typename Payload>
    void Connect(DelegateType d, Payload& payload) {
        payload_ = &payload;
        fn_ = d;
    }

    template <typename Payload>
    void Connect(DelegateType d, Payload* payload) {
        payload_ = payload;
        fn_ = d;
    }

    template <auto Func, typename Payload>
    void Connect(Payload& payload) {
        fn_ = wrap<Func>(payload, std::make_index_sequence<sizeof...(Args)>{});
    }

    template <auto Func, typename Payload>
    void Connect(Payload* payload) {
        fn_ = wrap<Func>(payload, std::make_index_sequence<sizeof...(Args)>{});
    }

    template <auto Func, size_t... Index>
    void Connect(std::index_sequence<Index...> indices) noexcept {
        fn_ = wrap<Func>(indices);
    }

    template <auto Func, typename Payload, size_t... Index>
    void Connect(Payload& payload, std::index_sequence<Index...> indices) {
        fn_ = wrap<Func>(payload, indices);
    }

    template <auto Func, typename Payload, size_t... Index>
    void Connect(Payload* payload, std::index_sequence<Index...> indices) {
        fn_ = wrap<Func>(payload, indices);
    }

    Ret operator()(Args... args) {
        return std::invoke(fn_, payload_, std::forward<Args>(args)...);
    }

    void Reset() {
        payload_ = nullptr;
        fn_ = nullptr;
    }

    operator bool() const {
        return fn_ != nullptr;
    }

    void Release() {
        payload_ = nullptr;
        fn_ = nullptr;
    }

    bool operator==(const Delegate& o) const {
        return o.payload_ == payload_ && o.fn_ == fn_;
    }

    bool operator!=(const Delegate& o) const {
        return !(*this == o);
    }

    ~Delegate() {
        Release();
    }

private:
    const void* payload_ = nullptr;
    DelegateType fn_ = nullptr; 

    template <auto Func, size_t... Index>
    DelegateType wrap(std::index_sequence<Index...>) {
        payload_ = nullptr;

        if constexpr (std::is_invocable_r_v<Ret, decltype(Func), Args...>) {
            return [](const void*, Args... args) -> Ret {
                auto forward_args = std::forward_as_tuple(std::forward<Args>(args)...);
                return static_cast<Ret>(std::invoke(Func,
                    std::forward<internal::type_list_elements_t<Index, internal::type_list<Args...>>>(std::get<Index>(forward_args))...));
            };
        } else {
            static_assert(false, "unsupport bind function, maybe a bug");
            return nullptr;
        }
    }

    template <auto Func, typename Payload, size_t... Index>
    DelegateType wrap(Payload& payload, std::index_sequence<Index...>) {
        payload_ = &payload;
        if constexpr (std::is_invocable_r_v<Ret, decltype(Func), Payload&, Args...>) {
            return [](const void* instance, Args... args) -> Ret {
                auto forward_args = std::forward_as_tuple(std::forward<Args>(args)...);
                return static_cast<Ret>(std::invoke(Func, *static_cast<Payload*>(const_cast<internal::constness_as_t<void, Payload>*>(instance)),
                    std::forward<internal::type_list_elements_t<Index, internal::type_list<Args...>>>(std::get<Index>(forward_args))...));
            };
        } else {
            static_assert(false, "unsupport bind function, maybe a bug");
            return nullptr;
        }
    }

    template <auto Func, typename Payload, size_t... Index>
    DelegateType wrap(Payload* payload, std::index_sequence<Index...>) {
        payload_ = payload;
        if constexpr (std::is_invocable_r_v<Ret, decltype(Func), Payload*, Args...>) {
            return [](const void* instance, Args... args) -> Ret {
                auto forward_args = std::forward_as_tuple(std::forward<Args>(args)...);
                return static_cast<Ret>(std::invoke(Func, static_cast<Payload*>(const_cast<internal::constness_as_t<void, Payload>*>(instance)),
                    std::forward<internal::type_list_elements_t<Index, internal::type_list<Args...>>>(std::get<Index>(forward_args))...));
            };
        } else {
            static_assert(false, "unsupport bind function, maybe a bug");
            return nullptr;
        }
    }   
};

}