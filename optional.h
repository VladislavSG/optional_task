#pragma once

#include <type_traits>
#include <utility>

struct nullopt_t {};
struct in_place_t {};
nullopt_t nullopt;
in_place_t in_place;


namespace optional_ {
    template <typename T, bool v = std::is_trivially_destructible_v<T>>
    struct destructible_base {
        constexpr destructible_base() {}

        constexpr explicit destructible_base(T value_)
            : value(std::move(value_)),
              is_valid(true) {}

        template<typename ...Args>
        constexpr destructible_base(in_place_t, Args&& ...args)
                : value(std::forward<Args>(args)...),
                  is_valid(true) {}

        ~destructible_base() {
            if (this->is_valid)
                this->value.~T();
        }

    protected:
        union {
            T value;
        };
        bool is_valid = false;
    };

    template <typename T>
    struct destructible_base<T, true> {
        constexpr destructible_base() : dummy() {}

        explicit constexpr destructible_base(T value_)
            : value(value_),
              is_valid(true) {}

        template<typename ...Args>
        constexpr destructible_base(in_place_t, Args&& ...args)
                : value(std::forward<Args>(args)...),
                  is_valid(true) {}
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

        constexpr optional_base(optional_base const& other) {
            if (other.is_valid) {
                new(&this->value) T(other.value);
                this->is_valid = true;
            }
        }

        constexpr optional_base(optional_base&& other) {
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
                if (this->is_valid) {
                    this->value.~T();
                    this->is_valid = false;
                }
            }
            return *this;
        }

        optional_base& operator=(optional_base&& other) {
            if (other.is_valid) {
                if (this->is_valid) {
                    this->value = std::move(other.value);
                } else {
                    new(&this->value) T(std::move(other.value));
                    this->is_valid = true;
                }
            } else {
                if (this->is_valid) {
                    this->value.~T();
                    this->is_valid = false;
                }
            }
            return *this;
        }
    };

    template <typename T>
    struct optional_base<T, true> : destructible_base<T> {
        using destructible_base<T>::destructible_base;
    };
}



template <typename T>
class optional : private optional_::optional_base<T> {
    using base = optional_::optional_base<T>;
public:
    using base::base;

    constexpr optional(nullopt_t) noexcept : optional() {};

    constexpr optional(optional const&) = default;
    constexpr optional(optional&&) = default;

    optional& operator=(optional const&) = default;
    optional& operator=(optional&&) = default;

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

    void reset() {
        if (this->is_valid) {
            this->value.~T();
            this->is_valid = false;
        }
    }
};

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