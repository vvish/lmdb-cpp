#pragma once

#include "cpp_lmdb/details/key_value_traits.hpp"
#include "cpp_lmdb/types.hpp"

#include <span>
#include <utility>

namespace lmdb
{

template <deserialization_trait KeyTrait, deserialization_trait ValueTrait>
class ro_db_item {
public:
    using key_type = typename KeyTrait::value_type;
    using value_type = typename ValueTrait::value_type;

    ro_db_item(byte_span const &key, byte_span const &value) noexcept
        : _key{key}, _value{value}
    {}

    auto key() const noexcept(noexcept(
        KeyTrait::from_bytes(std::declval<byte_span const &>()))) -> key_type
    {
        return KeyTrait::from_bytes(_key);
    }

    auto value() const noexcept(
        noexcept(ValueTrait::from_bytes(std::declval<byte_span const &>())))
        -> value_type
    {
        return ValueTrait::from_bytes(_value);
    }

private:
    byte_span _key;
    byte_span _value;
};

template <
    size_t Index,
    deserialization_trait KeyTrait,
    deserialization_trait ValueTrait>
std::tuple_element_t<Index, ro_db_item<KeyTrait, ValueTrait>> get(
    ro_db_item<KeyTrait, ValueTrait> const &item)
{
    if constexpr (Index == 0)
        return item.key();
    if constexpr (Index == 1)
        return item.value();
}

}  // namespace lmdb

namespace std
{

template <
    lmdb::deserialization_trait KeyTrait,
    lmdb::deserialization_trait ValueTrait>
struct tuple_size<lmdb::ro_db_item<KeyTrait, ValueTrait>>
    : integral_constant<size_t, 2> {};

template <
    lmdb::deserialization_trait KeyTrait,
    lmdb::deserialization_trait ValueTrait>
struct tuple_element<0, lmdb::ro_db_item<KeyTrait, ValueTrait>> {
    using type = typename KeyTrait::value_type;
};

template <
    lmdb::deserialization_trait KeyTrait,
    lmdb::deserialization_trait ValueTrait>
struct tuple_element<1, lmdb::ro_db_item<KeyTrait, ValueTrait>> {
    using type = typename ValueTrait::value_type;
};

}  // namespace std
