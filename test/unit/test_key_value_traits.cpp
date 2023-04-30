#include "cpp_lmdb/types.hpp"

// details
#include "cpp_lmdb/details/key_value_traits.hpp"

// std
#include <cstdint>
#include <span>
#include <vector>

namespace cpp_lmdb_tests
{
static_assert(lmdb::trivially_serealizable<int>);
static_assert(lmdb::trivially_serealizable<float>);
static_assert(!lmdb::trivially_serealizable<std::span<uint8_t const>>);
static_assert(!lmdb::trivially_serealizable<std::vector<uint8_t>>);

struct test_trait {
    using value_type = std::vector<uint8_t>;
    static auto to_bytes(value_type const &) -> lmdb::byte_span
    {
        return {};
    }
    static auto from_bytes(std::span<std::byte const> const &) -> value_type
    {
        return {};
    }

    static auto cmp(
        std::span<std::byte const> const &, std::span<std::byte const> const &)
        -> int
    {
        return {};
    }
};

struct empty_trait {};

static_assert(lmdb::serilaization_trait<test_trait>);
static_assert(lmdb::details::has_cmp_fun_v<test_trait>);
static_assert(lmdb::details::has_valid_key_order_params_v<test_trait>);
static_assert(lmdb::key_trait<test_trait>);

static_assert(!lmdb::serilaization_trait<empty_trait>);
static_assert(!lmdb::details::has_cmp_fun_v<empty_trait>);
static_assert(!lmdb::key_trait<empty_trait>);

struct test_reversed_key_trait : lmdb::trivial_trait<int> {
    using reversed_flag = std::true_type;
};

struct test_custom_cmp_fn_key_trait : lmdb::trivial_trait<int> {
    static auto cmp(value_type const &, value_type const &) -> int
    {
        return {};
    }
};

static_assert(lmdb::key_trait<test_custom_cmp_fn_key_trait>);
static_assert(lmdb::key_trait<test_reversed_key_trait>);

struct reversed_and_cmp_fn_trait {
    using reversed_flag = std::true_type;
    using value_type = std::vector<uint8_t>;
    static auto cmp(value_type const &, value_type const &) -> int
    {
        return {};
    }
};

static_assert(!lmdb::key_trait<reversed_and_cmp_fn_trait>);
static_assert(!lmdb::value_trait<reversed_and_cmp_fn_trait>);

struct fixed_and_cmp_fn_value_trait {
    using fixed_size_flag = std::true_type;
    using value_type = std::vector<uint8_t>;
    static auto cmp(value_type const &, value_type const &) -> int
    {
        return {};
    }
};

static_assert(!lmdb::value_trait<fixed_and_cmp_fn_value_trait>);

static_assert(
    lmdb::details::has_valid_key_order_params_v<lmdb::trivial_trait<int>>);

}  // namespace cpp_lmdb_tests
