#pragma once

#include <type_traits>

namespace kl {

template <typename Enum>
class enum_set
{
    static_assert(std::is_enum<Enum>::value, "Enum is not a enum-type");

public:
    using underlying_type = std::underlying_type_t<Enum>;
    using enum_type = Enum;

public:
    constexpr enum_set() noexcept : value_{} {}
    constexpr explicit enum_set(Enum value) noexcept : value_{value} {}

    constexpr underlying_type underlying_value() const noexcept
    {
        return static_cast<underlying_type>(value_);
    }

    constexpr Enum value() const noexcept { return value_; }

    constexpr explicit operator bool() const noexcept
    {
        return underlying_value() != 0;
    }

    constexpr bool test(Enum value) const noexcept { return has_all(value); }

    constexpr bool has_any(Enum value) const noexcept
    {
        return has_any(enum_set{value});
    }

    constexpr bool has_any(enum_set other) const noexcept
    {
        return (*this & other) != enum_set{};
    }

    constexpr bool has_all(Enum value) const noexcept
    {
        return has_all(enum_set{value});
    }

    constexpr bool has_all(enum_set other) const noexcept
    {
        return (*this & other) == other;
    }

    constexpr enum_set operator~() const noexcept
    {
        return enum_set{static_cast<Enum>(~underlying_value())};
    }

    constexpr enum_set operator&(enum_set other) const noexcept
    {
        return enum_set{
            static_cast<Enum>(underlying_value() & other.underlying_value())};
    }

    constexpr enum_set operator|(enum_set other) const noexcept
    {
        return enum_set{
            static_cast<Enum>(underlying_value() | other.underlying_value())};
    }

    constexpr enum_set operator^(enum_set other) const noexcept
    {
        return enum_set{
            static_cast<Enum>(underlying_value() ^ other.underlying_value())};
    }

    constexpr enum_set& operator&=(enum_set other) noexcept
    {
        value_ =
            static_cast<Enum>(underlying_value() & other.underlying_value());
        return *this;
    }

    constexpr enum_set& operator|=(enum_set other) noexcept
    {
        value_ =
            static_cast<Enum>(underlying_value() | other.underlying_value());
        return *this;
    }

    constexpr enum_set& operator^=(enum_set other) noexcept
    {
        value_ =
            static_cast<Enum>(underlying_value() ^ other.underlying_value());
        return *this;
    }

    constexpr bool operator==(enum_set other) const noexcept
    {
        return value_ == other.value_;
    }

    constexpr bool operator<(enum_set other) const noexcept
    {
        return value_ < other.value_;
    }

    constexpr bool operator!=(enum_set other) const noexcept
    {
        return !(*this == other);
    }

    constexpr enum_set operator&(Enum value) const noexcept
    {
        return (*this & enum_set{value});
    }

    constexpr enum_set operator|(Enum value) const noexcept
    {
        return (*this | enum_set{value});
    }

    constexpr enum_set operator^(Enum value) const noexcept
    {
        return (*this ^ enum_set{value});
    }

    constexpr enum_set& operator&=(Enum value) noexcept
    {
        return (*this &= enum_set{value});
    }

    constexpr enum_set& operator|=(Enum value) noexcept
    {
        return (*this |= enum_set{value});
    }

    constexpr enum_set& operator^=(Enum value) noexcept
    {
        return (*this ^= enum_set{value});
    }

    constexpr bool operator==(Enum value) const noexcept
    {
        return (*this == enum_set{value});
    }

    constexpr bool operator<(Enum value) const noexcept
    {
        return (*this < enum_set{value});
    }

    constexpr bool operator!=(Enum value) const noexcept
    {
        return (*this != enum_set{value});
    }

    constexpr friend enum_set operator|(Enum a, enum_set b) noexcept
    {
        return enum_set{a} | b;
    }

    constexpr friend enum_set operator&(Enum a, enum_set b) noexcept
    {
        return enum_set{a} & b;
    }

    constexpr friend enum_set operator^(Enum a, enum_set b) noexcept
    {
        return enum_set{a} ^ b;
    }

    constexpr friend bool operator==(Enum a, enum_set b) noexcept
    {
        return enum_set{a} == b;
    }

    constexpr friend bool operator<(Enum a, enum_set b) noexcept
    {
        return enum_set{a} < b;
    }

    constexpr friend bool operator!=(Enum a, enum_set b) noexcept
    {
        return enum_set{a} != b;
    }

private:
    Enum value_;
};

template <typename T>
struct is_enum_set : std::false_type
{
};

template <typename T>
struct is_enum_set<enum_set<T>> : std::true_type
{
};

template <typename T>
inline constexpr bool is_enum_set_v = is_enum_set<T>::value;
} // namespace kl
