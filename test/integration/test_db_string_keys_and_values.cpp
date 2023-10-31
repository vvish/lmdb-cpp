#include "cpp_lmdb/cpp_lmdb.hpp"

#include "test_utils.hpp"

// gtest
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// std
#include <cstring>
#include <filesystem>

using namespace ::testing;  // NOLINT(google-build-using-namespace)

namespace cpp_lmdb_tests
{

using test_trait = lmdb::
    unique_key<lmdb::string_trait, lmdb::string_trait>;

using ro_view
    = lmdb::ro_environment<>::ro_db<test_trait>::ro_transaction::ro_view;


TEST(integration_test, db_string_keys_and_values_unique_key)
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

    EXPECT_TRUE(transaction->insert("A", "ABC"));
    EXPECT_TRUE(transaction->insert("B", "DEF"));
    {
        auto const result = transaction->try_insert("A", "FGH");
        ASSERT_FALSE(result);
        EXPECT_EQ(result.error(), lmdb::error_t::key_exist);
    }
    {
        const auto result = transaction->get("A");
        ASSERT_TRUE(result);
        EXPECT_EQ(*result, "ABC");
    }
    {
        const auto result = transaction->get("B");
        ASSERT_TRUE(result);
        EXPECT_EQ(*result, "DEF");
    }

    {
        ASSERT_TRUE(transaction->insert("B", "JKL"));
        const auto result = transaction->get("B");
        ASSERT_TRUE(result);
        EXPECT_EQ(*result, "JKL");
    }

    rw_db->commit_transaction(std::move(*transaction));
    {
        auto ro_tx = rw_db->begin_ro_transaction();
        ASSERT_TRUE(ro_tx);
        {
            const auto result = ro_tx->get("A");
            ASSERT_TRUE(result);
            EXPECT_EQ(*result, "ABC");
        }
        {
            const auto result = ro_tx->get("B");
            ASSERT_TRUE(result);
            EXPECT_EQ(*result, "JKL");
        }
    }
    {
        auto ro_tx = rw_db->begin_ro_transaction();
        ASSERT_TRUE(ro_tx);
        EXPECT_THAT(
            cpp_lmdb_tests::get_all_values(ro_tx->iterate().value()),
            ElementsAre("ABC", "JKL"));
    }
}

}  // namespace cpp_lmdb_tests
