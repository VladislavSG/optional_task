#pragma once

#include <type_traits>
#include <utility>

struct nullopt_t {};
struct in_place_t {};
inline constexpr nullopt_t nullopt;
inline constexpr in_place_t in_place;


namespace optional_ {
    template <typename T, bool v = std::is_trivially_destructible_v<T>>
    struct destructible_base {
        constexpr destructible_base() noexcept {}

        constexpr explicit destructible_base(T value_)
            : value(std::move(value_)),
              is_valid(true) {}

        template<typename ...Args>
        constexpr destructible_base(in_place_t, Args&& ...args)
                : value(std::forward<Args>(args)...),
                  is_valid(true) {}

        ~destructible_base() {
            reset();
        }

        void reset() {
            if (is_valid) {
                value.~T();
                is_valid = false;
            }
        }

    protected:
        union {
            T value;
            char dummy;
        };
        bool is_valid = false;
    };

    template <typename T>
    struct destructible_base<T, true> {
        constexpr destructible_base() noexcept
            : dummy() {}

        explicit constexpr destructible_base(T value_)
            : value(std::move(value_)),
              is_valid(true) {}

        template<typename ...Args>
        constexpr destructible_base(in_place_t, Args&& ...args)
                : value(std::forward<Args>(args)...),
                  is_valid(true) {}

        void reset() {
            is_valid = false;
        }
    protected:
        union {
            T value;
            char dummy;
        };
        bool is_valid = false;
    };

    template <typename T, bool v = std::is_trivially_copyable_v<T>>
    struct optional_base : destructible_base<T> {
        using destructible_base<T>::destructible_base;
        using destructible_base<T>::reset;

        optional_base(optional_base const& other) {
            if (other.is_valid) {
                new(&this->value) T(other.value);
                this->is_valid = true;
            }
        }

        optional_base(optional_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
            if (other.is_valid) {
                new(&this->value) T(std::move(other.value));
                this->is_valid = true;
            }
        }

        optional_base& operator=(optional_base const& other) {
            if (other.is_valid) {
                if (this->is_valid) {
                    this->value = other.value;
                } else {
                    new(&this->value) T(other.value);
                    this->is_valid = true;
                }
            } else {
                reset();
            }
            return *this;
        }

        optional_base& operator=(optional_base&& other) noexcept(
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>) {
            if (other.is_valid) {
                if (this->is_valid) {
                    this->value = std::move(other.value);
                } else {
                    new(&this->value) T(std::move(other.value));
                    this->is_valid = true;
                }
            } else {
                reset();
            }
            return *this;
        }
    };

    template <typename T>
    struct optional_base<T, true> : destructible_base<T> {
        using destructible_base<T>::destructible_base;
        using destructible_base<T>::reset;
    };
}



template <typename T>
class optional : private optional_::optional_base<T> {
    using base = optional_::optional_base<T>;
public:
    using base::base;
    using base::reset;

    constexpr optional(nullopt_t) noexcept : optional() {};

    constexpr optional(optional const&) = default;
    constexpr optional(optional&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

    optional& operator=(optional const&) = default;
    optional& operator=(optional&&) noexcept(std::is_nothrow_move_constructible_v<T>
            && std::is_nothrow_move_assignable_v<T>) = default;

    optional& operator=(nullopt_t) noexcept {
        reset();
        return *this;
    }

    constexpr explicit operator bool() const noexcept {
        return this->is_valid;
    }

    constexpr T& operator*() noexcept {
        return this->value;
    }
    constexpr T const& operator*() const noexcept {
        return this->value;
    }

    constexpr T* operator->() noexcept {
        return &this->value;
    }
    constexpr T const* operator->() const noexcept {
        return &this->value;
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        reset();
        new(&this->value) T(std::forward<Args>(args)...);
        this->is_valid = true;
    }
};

template<typename T>
constexpr bool operator==(optional<T> const &a, optional<T> const &b) {
    if (static_cast<bool>(a) != static_cast<bool>(b)) {
        return false;
    } else if (static_cast<bool>(a)) {
        return *a == *b;
    } else {
        return true;
    }
}

template<typename T>
constexpr bool operator!=(optional<T> const &a, optional<T> const &b) {
    if (static_cast<bool>(a) != static_cast<bool>(b)) {
        return true;
    } else if (static_cast<bool>(a)) {
        return *a != *b;
    } else {
        return false;
    }
}

template<typename T>
constexpr bool operator<(optional<T> const &a, optional<T> const &b) {
    if (static_cast<bool>(a) != static_cast<bool>(b)) {
        return static_cast<bool>(b);
    } else if (static_cast<bool>(a)) {
        return *a < *b;
    } else {
        return false;
    }
}

template<typename T>
constexpr bool operator<=(optional<T> const &a, optional<T> const &b) {
    if (static_cast<bool>(a) != static_cast<bool>(b)) {
        return static_cast<bool>(b);
    } else if (static_cast<bool>(a)) {
        return *a <= *b;
    } else {
        return true;
    }
}

template<typename T>
constexpr bool operator>(optional<T> const &a, optional<T> const &b) {
    if (static_cast<bool>(a) != static_cast<bool>(b)) {
        return static_cast<bool>(a);
    } else if (static_cast<bool>(a)) {
        return *a > *b;
    } else {
        return false;
    }
}

template<typename T>
constexpr bool operator>=(optional<T> const &a, optional<T> const &b) {
    if (static_cast<bool>(a) != static_cast<bool>(b)) {
        return static_cast<bool>(a);
    } else if (static_cast<bool>(a)) {
        return *a >= *b;
    } else {
        return true;
    }
}
