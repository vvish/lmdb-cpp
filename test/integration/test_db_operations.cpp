#include "cpp_lmdb/cpp_lmdb.hpp"

// gtest
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// std
#include <cstring>
#include <filesystem>

using namespace ::testing;  // NOLINT(google-build-using-namespace)

namespace cpp_lmdb_tests
{

template <std::ranges::range R>
auto to_vector(R &&range)
{
    std::vector<std::ranges::range_value_t<R>> result;
    std::ranges::copy(std::forward<R>(range), std::back_inserter(result));

    return result;
}

using test_trait = lmdb::
    unique_key<lmdb::trivial_trait<uint8_t>, lmdb::trivial_trait<uint32_t>>;

using ro_view
    = lmdb::ro_environment<>::ro_db<test_trait>::ro_transaction::ro_view;

auto get_all_values(ro_view const &view)
{
    auto const values
        = std::ranges::ref_view{view}
          | std::views::transform([](auto const &it) { return it.value(); });

    return to_vector(values);
}

TEST(integration_test, db_operations_unique_key)
{
    constexpr auto test_env = "./test_env";

    if (std::filesystem::exists(test_env))
        std::filesystem::remove_all(test_env);
    std::filesystem::create_directory(test_env);

    auto environment = lmdb::make_environment<lmdb::env_flags_t::none, 1>(
        test_env, lmdb::default_file_mode);

    ASSERT_TRUE(environment);
    auto rw_db = environment->open_rw_db<test_trait>(
        "test_db", lmdb::create_if_not_exists::yes);

    ASSERT_TRUE(rw_db);

    auto transaction = rw_db->begin_rw_transaction();
    ASSERT_TRUE(transaction);

    EXPECT_TRUE(transaction->insert(0xAA, 2000));
    EXPECT_TRUE(transaction->insert(0xBB, 5000));
    {
        auto const result = transaction->try_insert(0xAA, 3000);
        ASSERT_FALSE(result);
        EXPECT_EQ(result.error(), lmdb::error_t::key_exist);
    }
    {
        const auto result = transaction->get(0xAA);
        ASSERT_TRUE(result);
        EXPECT_EQ(*result, 2000);
    }
    {
        const auto result = transaction->get(0xBB);
        ASSERT_TRUE(result);
        EXPECT_EQ(*result, 5000);
    }

    {
        ASSERT_TRUE(transaction->insert(0xBB, 10000));
        const auto result = transaction->get(0xBB);
        ASSERT_TRUE(result);
        EXPECT_EQ(*result, 10000);
    }

    rw_db->commit_transaction(std::move(*transaction));
    {
        auto ro_tx = rw_db->begin_ro_transaction();
        ASSERT_TRUE(ro_tx);
        {
            const auto result = ro_tx->get(0xAA);
            ASSERT_TRUE(result);
            EXPECT_EQ(*result, 2000);
        }
        {
            const auto result = ro_tx->get(0xBB);
            ASSERT_TRUE(result);
            EXPECT_EQ(*result, 10000);
        }
    }
    {
        auto ro_tx = rw_db->begin_ro_transaction();
        ASSERT_TRUE(ro_tx);
        {
            auto const cursor = ro_tx->iterate();
            auto values = std::ranges::ref_view{*cursor}
                          | std::views::transform(
                              [](auto const &it) { return it.value(); });

            EXPECT_THAT(to_vector(values), ElementsAre(2000, 10000));

            EXPECT_THAT(
                get_all_values(ro_tx->iterate().value()),
                ElementsAre(2000, 10000));
        }
    }
}

}  // namespace cpp_lmdb_tests
