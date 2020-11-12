#pragma once

#include <type_traits>
#include <utility>

struct nullopt_t {};

struct in_place_t {};

nullopt_t nullopt;
in_place_t in_place;

namespace optional_ {
    template <typename T> //TODO delete?
    inline constexpr bool is_td = std::is_trivially_destructible_v<T>;

    template <typename T>
    inline constexpr bool is_tc = std::is_trivially_copyable_v<T>;

    template <typename T>
    struct storage {


    private:
        std::aligned_storage<sizeof(T), alignof(T)> data;
        bool is_valid = false;
    };

    template <typename T, bool v = is_td<T>>
    struct destructible_base {


        destructible_base& operator=(destructible_base const& other) {
            if (other.is_valid) {
                if (is_valid) {
                    value = other.value;
                } else {
                    value = T(other.value); //TODO
                }
            }
        }

        ~destructible_base() {
            if (is_valid)
                value.~T();
        }

        union {
            T value;
            char dummy; //TODO try = '0' without constructor initialize
        };
        bool is_valid;
    };

    template <typename T>
    struct destructible_base<T, true> {
        union {
            T value;
            char dummy; //TODO try = '0' without constructor initialize
        };
        bool is_valid;
    };

    template <typename T, bool v = is_tc<T>>
    struct optional_base : destructible_base<T> {
        using destructible_base<T>::is_valid;
        using destructible_base<T>::value;

        optional_base(optional_base const& other) {
            if (is_valid) {
                value.T(other.value);
            }
        };

        optional_base() = default;
        ~optional_base() = default;
    };

    template <typename T>
    struct optional_base<T, true> : destructible_base<T> {};
}



template <typename T>
class optional : private optional_::optional_base<T> {
    using base = optional_::optional_base<T>;
    using base::value;
    using base::is_valid;

public:
    constexpr optional() noexcept {};
    constexpr optional(nullopt_t) noexcept : optional() {};

    constexpr optional(optional const&) = default;
    constexpr optional(optional&&) = default;

    optional& operator=(optional const&) = default;
    optional& operator=(optional&&) = default;

    constexpr optional(T value) {
        value = std::move(value);
    };

    template <typename... Args>
    explicit constexpr optional(in_place_t, Args&&... args)
            : optional_::optional_base<T>(std::forward<Args>(args)...) {}

    optional& operator=(nullopt_t) noexcept {
        reset();
    }

    constexpr explicit operator bool() const noexcept {
        return is_valid;
    }

    constexpr T& operator*() noexcept {
        return value;
    }
    constexpr T const& operator*() const noexcept {
        return value;
    }

    constexpr T* operator->() noexcept {
        return &value;
    }
    constexpr T const* operator->() const noexcept {
        return &value;
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        reset();
        value = T(std::forward<Args>(args)...);
        is_valid = true;
    }

    void reset() {
        if (is_valid) {
            value.~T();
            is_valid = false;
        }
    }
};

//TODO throw exception if not initialize?
template<typename T>
constexpr bool operator==(optional<T> const &a, optional<T> const &b) {
    return *a == *b;
}

template<typename T>
constexpr bool operator!=(optional<T> const &a, optional<T> const &b) {
    return *a != *b;
}

template<typename T>
constexpr bool operator<(optional<T> const &a, optional<T> const &b) {
    return (*a) < (*b);
}

template<typename T>
constexpr bool operator<=(optional<T> const &a, optional<T> const &b) {
    return *a <= *b;
}

template<typename T>
constexpr bool operator>(optional<T> const &a, optional<T> const &b) {
    return *a > *b;
}

template<typename T>
constexpr bool operator>=(optional<T> const &a, optional<T> const &b) {
    return *a >= *b;
}