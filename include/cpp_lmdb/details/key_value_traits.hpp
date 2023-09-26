#pragma once

// lmdb
#include "lmdb.h"

// std
#include <concepts>
#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <type_traits>

namespace lmdb
{
namespace details
{
template <typename T>
constexpr bool has_cmp_fun_v = requires {
    {
        T::cmp(
            std::declval<std::span<std::byte const> const &>(),
            std::declval<std::span<std::byte const> const &>())
    } -> std::same_as<int>;
};

template <typename T>
inline constexpr bool has_reversed_flag_v = requires { typename T::reversed_flag; };

constexpr auto logical_nand(bool const lhs, bool const rhs) -> bool
{
    return !(lhs && rhs);
}

template <typename T>
inline constexpr bool has_valid_key_order_params_v
    = logical_nand(has_reversed_flag_v<T>, has_cmp_fun_v<T>);

template <typename T>
inline constexpr bool has_fixed_value_size_flag_v
    = requires { typename T::fixed_size_flag; };

template <typename T>
inline constexpr bool has_valid_value_order_params_v = logical_nand(
    (has_reversed_flag_v<T> || has_fixed_value_size_flag_v<T>),
    has_cmp_fun_v<T>);

template <typename T>
constexpr bool get_duplicate_key_flag()
{
    if constexpr (requires { typename T::duplicate_key_flag; })
        return T::duplicate_key_flag::value;
    else
        return false;
}

template <typename T>
constexpr bool get_reversed_flag()
{
    if constexpr (has_reversed_flag_v<T>)
        return T::reversed_flag::value;
    else
        return false;
}

template <typename T>
constexpr bool get_fixed_value_size_flag()
{
    if constexpr (has_fixed_value_size_flag_v<T>)
        return T::fixed_size_flag::value;
    else
        return false;
}

template <typename T>
inline constexpr bool is_integer_type_v
    = std::is_same_v<T, unsigned int> || std::is_same_v<T, size_t>;

}  // namespace details

template <typename T>
concept span_like = requires(T t) {
    typename T::value_type;
    typename T::pointer;
    {
        t.data()
    } -> std::same_as<typename T::pointer>;
    {
        t.size()
    } -> std::same_as<size_t>;
};

template <typename T>
concept byte_span_like
    = span_like<T> && std::is_same_v<typename T::value_type, std::byte>;

template <typename T>
concept trivially_serealizable
    = std::is_trivially_copyable_v<T> && !span_like<T>;

template <typename T>
concept serilaization_trait = requires {
    typename T::value_type;
    {
        T::to_bytes(std::declval<typename T::value_type const &>())
    } -> byte_span_like;
};

template <typename T>
concept deserialization_trait = requires {
    typename T::value_type;
    {
        T::from_bytes(std::declval<std::span<std::byte const>>())
    } -> std::same_as<typename T::value_type>;
};

template <typename T>
concept key_trait = serilaization_trait<T> && deserialization_trait<T>
                    && details::has_valid_key_order_params_v<T>;

template <typename T>
concept value_trait
    = serilaization_trait<T> && details::has_valid_value_order_params_v<T>;

template <trivially_serealizable T>
struct trivial_trait {
    using value_type = T;
    static constexpr auto to_bytes(value_type const &value)
        -> std::span<std::byte const>
    {
        return std::as_bytes(
            std::span<value_type const, 1>{std::addressof(value), 1});
    }

    static constexpr auto from_bytes(std::span<std::byte const> const &bytes)
        -> value_type
    {
        value_type value{};
        std::memcpy(&value, bytes.data(), sizeof(value_type));
        return value;
    }
};

struct string_trait {
    using value_type = std::string;
    static auto to_bytes(std::string const &value)
        -> std::span<std::byte const>
    {
        return std::as_bytes(std::span{value.data(), value.size()});
    }

    static auto from_bytes(std::span<std::byte const> const &bytes)
        -> std::string
    {
        return std::string{
            reinterpret_cast<char const *>(bytes.data()), bytes.size()};
    }
};

template <key_trait K, value_trait V>
struct basic_key {
    using key_trait = K;
    using value_trait = V;
};

template <typename T>
concept key_value_trait
    = key_trait<typename T::key_trait> && value_trait<typename T::value_trait>;

template <key_trait K, value_trait V>
struct unique_key : basic_key<K, V> {};

template <key_trait K, value_trait V>
struct duplicate_key : basic_key<K, V> {
    using duplicate_key_flag = std::true_type;
};

namespace details
{
template <key_value_trait KV>
struct key_value_trait_helper {
    using key_trait = typename KV::key_trait;
    using value_trait = typename KV::value_trait;

    using key_type = typename key_trait::value_type;
    using value_type = typename value_trait::value_type;

    constexpr static bool duplicates_enabled = get_duplicate_key_flag<KV>();
    constexpr static bool key_reversed = get_reversed_flag<key_trait>();
    constexpr static bool value_reversed = get_reversed_flag<value_trait>();
    constexpr static bool value_fixed_size
        = get_fixed_value_size_flag<value_trait>();

    constexpr static bool is_integer_key = is_integer_type_v<key_type>;
    constexpr static bool is_integer_value = is_integer_type_v<value_type>;

    constexpr static bool has_key_cmp_fun = has_cmp_fun_v<key_trait>;
    constexpr static bool has_value_cmp_fun = has_cmp_fun_v<value_trait>;

    constexpr static auto db_key_value_flags() -> unsigned int
    {
        unsigned int flags{};
        if (duplicates_enabled)
            flags |= MDB_DUPSORT;
        if (key_reversed)
            flags |= MDB_REVERSEKEY;
        if (value_reversed)
            flags |= MDB_REVERSEDUP;
        if (is_integer_key)
            flags |= MDB_INTEGERKEY;
        if (is_integer_value)
            flags |= MDB_INTEGERDUP;

        return flags;
    }
};
}  // namespace details
}  // namespace lmdb
