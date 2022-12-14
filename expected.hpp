/* this is a C++17 implementation for std::unexpected and std::expected(C++23)
 * [reference](https://en.cppreference.com/w/cpp/utility/expected) */

#pragma once

#include <type_traits>
#include <utility>
#include <variant>
#include <exception>
#include <stdexcept>
#include <optional>

namespace gmq {

template<typename E>
class bad_expected_access;

template<>
class bad_expected_access<void> : public std::exception {
    void error() {}
};

template<typename E>
class bad_expected_access: bad_expected_access<void> {
public:
    bad_expected_access(const E& error): error_(error) {}

    const E& error() const& noexcept { return error_; }
    E& error() & noexcept { return error_; }
    const E&& error() const&& noexcept { return std::move(error_); }
    E&& error() && noexcept { return std::move(error_); }

private:
    E error_;
};


template <typename E>
class unexpected final {
public:
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) = default;

    template <typename Err = E>
    constexpr explicit unexpected(std::enable_if_t<std::is_convertible_v<Err, E>, Err>&& e): error_(std::forward<Err>(e)) {}

    template <typename Err = E>
    constexpr explicit unexpected(const std::enable_if_t<std::is_convertible_v<Err, E>, Err>& e): error_(e) {}

    /*
    template<typename... Args >
    constexpr explicit unexpected(std::in_place_t, Args&&... args) {
        // TOOD
    } 

    template<typename U, typename... Args >
    constexpr explicit unexpected(std::in_place_t, std::initializer_list<U> il, Args&&... args) {
        // TODO
    }
    */

    constexpr const E& error() const& noexcept { return error_; }
    constexpr E& error() & noexcept { return error_; }
    constexpr const E&& error() const&& noexcept  { return std::move(error_); }
    constexpr  E&& error() && noexcept  { return std::move(error_); }

    constexpr void swap(unexpected& e) noexcept(std::is_nothrow_swappable_v<E>) { std::swap(e, error_); }

    constexpr bool operator==(const unexpected& o) const {
        return o.error() == error();
    }

private:
    E error_;
};


template <typename T, typename E>
class expected {
public:
    using value_type = std::enable_if_t<std::is_destructible_v<T> &&
                                        std::is_copy_constructible_v<T> &&
                                        std::is_move_constructible_v<T>, T>;
    using error_type = E;

    template <typename U>
    using rebind = expected<value_type, error_type>;

    constexpr explicit expected(const T& value): data_(value) {}
    constexpr explicit expected(T&& value): data_(std::move(value)) {}
    constexpr explicit expected(const unexpected<error_type>& error): data_(error.error()) {}
    constexpr explicit expected(unexpected<error_type>&& err): data_(std::move(err.error())) {}

    constexpr const value_type& value() const& {
        auto value = std::get_if<value_type>(data_);
        if (value) {
            return *value;
        } else {
            throw bad_expected_access(std::get<error_type>(data_));
        }
    }

    constexpr value_type& value() & {
        auto value = std::get_if<value_type>(&data_);
        if (value) {
            return *value;
        } else {
            throw bad_expected_access(std::get<error_type>(data_));
        }
    }

    constexpr const value_type&& value() const&& {
        auto value = std::get_if<value_type>(&data_);
        if (value) {
            return std::move(*value);
        } else {
            throw bad_expected_access(std::get<error_type>(data_));
        }
    }

    constexpr value_type&& value() && {
        auto value = std::get_if<value_type>(&data_);
        if (value) {
            return std::move(*value);
        } else {
            throw bad_expected_access(std::get<error_type>(data_));
        }
    }

    constexpr const error_type& error() const& noexcept {
        return std::get<error_type>(data_);
    }

    constexpr error_type& error() & noexcept {
        return std::get<error_type>(data_);
    }

    constexpr const error_type&& error() const&& noexcept {
        return std::move(std::get<error_type>(data_));
    }

    constexpr error_type&& error() && noexcept {
        return std::move(std::get<error_type>(data_));
    }

    operator bool() const {
        return has_value();
    }

    bool has_value() const {
        return std::get_if<value_type>(&data_);
    }

    template <typename U>
    constexpr T value_or(U&& default_value) const& {
        return bool(*this) ? **this : static_cast<T>(std::forward<U>(default_value));
    }

    template <typename U>
    constexpr T value_or(U&& default_value) && {
        return bool(*this) ? std::move(**this) : static_cast<T>(std::forward<U>(default_value));
    }

    expected& operator=(const expected&) = default;
    expected& operator=(const unexpected<E>& error) {
        data_ = error.error();
    }

    expected& operator=( unexpected<E>&& error) {
        data_ = std::move(error.error());
        return *this;
    }

private:
    std::variant<value_type, error_type> data_;
};

template <typename E>
class expected<void, E> {
public:
    using value_type = void;
    using error_type = E;

    template <typename U>
    using rebind = expected<U, error_type>;

    constexpr expected() = default;
    constexpr explicit expected(const E& e): error_(e) {}

    template <typename U>
    constexpr explicit expected(const std::enable_if_t<std::is_convertible_v<U, E>>& e): error_(e) {}

    constexpr expected(const unexpected<E>& err): error_(err.error()) {}
    constexpr expected(unexpected<E>&& err): error_(std::move(err.error())) {}

    constexpr value_type value() const {}
    constexpr value_type value() {}

    operator bool() const {
        return has_value();
    }

    bool has_value() const {
        return !error_.has_value();
    }

    template <typename U>
    constexpr U value_or(U&& default_value) const& {
        return std::forward(default_value);
    }

    template <typename U>
    constexpr U value_or(U&& default_value) && {
        return std::forward(default_value);
    }

    constexpr const error_type& error() const& noexcept {
        return error_.value();
    }

    constexpr error_type& error() & noexcept {
        return error_.value();
    }

    constexpr const error_type&& error() const&& noexcept {
        return std::move(error_.value());
    }

    constexpr error_type&& error() && noexcept {
        return std::move(error_.value());
    }

    expected& operator=(const expected&) = default;
    expected& operator=(const unexpected<E>& error) {
        error_ = error.error();
    }

    expected& operator=( unexpected<E>&& error) {
        error_ = std::move(error.error());
    }

private:
    std::optional<error_type> error_;
};

}
