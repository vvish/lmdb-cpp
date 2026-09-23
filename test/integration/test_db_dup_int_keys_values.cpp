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

TEST(
    integration_test,
    db_int_duplicate_keys_and_values_iterate_by_key_lower_bound)
{
    using test_trait = lmdb::duplicate_key<
        lmdb::trivial_trait<uint8_t>,
        lmdb::trivial_trait<uint32_t>>;

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
    EXPECT_TRUE(transaction->insert(0xBB, 5001));
    EXPECT_TRUE(transaction->insert(0xBB, 5002));
    EXPECT_TRUE(transaction->insert(0xCC, 10000));

    ASSERT_TRUE(rw_db->commit_transaction(std::move(*transaction)));
    {
        auto const ro_tx = rw_db->begin_ro_transaction();
        ASSERT_TRUE(ro_tx);

        EXPECT_THAT(
            cpp_lmdb_tests::get_all_values(ro_tx->lower_bound(0xBB).value()),
            ElementsAre(5001, 5002, 10000));
    }
}

}  // namespace cpp_lmdb_tests
