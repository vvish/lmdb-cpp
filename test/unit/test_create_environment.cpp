#include "cpp_lmdb/cpp_lmdb.hpp"
#include "mocks.hpp"

// gtest
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// std
#include <concepts>

namespace cpp_lmdb_tests
{
using namespace ::testing;  // NOLINT(google-build-using-namespace)

template <typename T, typename DbTrait>
constexpr bool env_has_open_rw_db_v = requires(T t) {
    t.template open_rw_db<DbTrait>(std::declval<char const *>());
};

using test_db_trait = lmdb::unique_key<
    lmdb::trivial_trait<int>,
    lmdb::trivial_trait<int>>;

TEST(test_create_environment, environment_created_and_released)
{
    StrictMock<mocks::api> api;

    MDB_env *test_env{reinterpret_cast<MDB_env *>(0x42)};

    constexpr auto test_env_path = "env path";
    constexpr size_t test_max_db_count{1};
    constexpr lmdb::db_file_mode_t test_file_mode{0x42};

    {
        InSequence seq;

        EXPECT_CALL(api, mdb_env_create(_))
            .WillOnce(DoAll(SetArgPointee<0>(test_env), Return(MDB_SUCCESS)));
        EXPECT_CALL(api, mdb_env_set_maxdbs(test_env, test_max_db_count));
        EXPECT_CALL(
            api,
            mdb_env_open(test_env, StrEq(test_env_path), 0, test_file_mode))
            .WillOnce(Return(MDB_SUCCESS));
        EXPECT_CALL(api, mdb_env_close(test_env));
    }

    auto const env
        = lmdb::make_environment<lmdb::env_flags_t::none, test_max_db_count>(
            test_env_path, test_file_mode, api);

    static_assert(
        env_has_open_rw_db_v<decltype(env)::value_type, test_db_trait>);
}

TEST(test_create_environment, ro_environment_created_and_released)
{
    StrictMock<mocks::api> api;

    MDB_env *test_env{reinterpret_cast<MDB_env *>(0x42)};

    constexpr auto test_env_path = "env path";
    constexpr size_t test_max_db_count{1};
    constexpr lmdb::db_file_mode_t test_file_mode{0x42};

    {
        InSequence seq;

        EXPECT_CALL(api, mdb_env_create(_))
            .WillOnce(DoAll(SetArgPointee<0>(test_env), Return(MDB_SUCCESS)));
        EXPECT_CALL(api, mdb_env_set_maxdbs(test_env, test_max_db_count));
        EXPECT_CALL(
            api,
            mdb_env_open(
                test_env, StrEq(test_env_path), MDB_RDONLY, test_file_mode))
            .WillOnce(Return(MDB_SUCCESS));
        EXPECT_CALL(api, mdb_env_close(test_env));
    }

    constexpr size_t num_dbs{1};

    auto const env
        = lmdb::make_environment<lmdb::env_flags_t::read_only, num_dbs>(
            test_env_path, test_file_mode, api);

    static_assert(
        !env_has_open_rw_db_v<decltype(env)::value_type, test_db_trait>);
}

TEST(test_create_environment, ro_no_lock_environment_created_and_released)
{
    StrictMock<mocks::api> api;

    MDB_env *test_env{reinterpret_cast<MDB_env *>(0x42)};

    constexpr auto test_env_path = "env path";
    constexpr size_t test_max_db_count{1};
    constexpr lmdb::db_file_mode_t test_file_mode{0x42};

    {
        InSequence seq;

        EXPECT_CALL(api, mdb_env_create(_))
            .WillOnce(DoAll(SetArgPointee<0>(test_env), Return(MDB_SUCCESS)));
        EXPECT_CALL(api, mdb_env_set_maxdbs(test_env, test_max_db_count));
        EXPECT_CALL(
            api,
            mdb_env_open(
                test_env,
                StrEq(test_env_path),
                MDB_RDONLY | MDB_NOLOCK,
                test_file_mode))
            .WillOnce(Return(MDB_SUCCESS));
        EXPECT_CALL(api, mdb_env_close(test_env));
    }

    auto const env = lmdb::make_environment<
        lmdb::env_flags_t::read_only | lmdb::env_flags_t::no_lock,
        1>(test_env_path, test_file_mode, api);

    static_assert(
        !env_has_open_rw_db_v<decltype(env)::value_type, test_db_trait>);
}

TEST(test_create_environment, create_failed)
{
    StrictMock<mocks::api> api;

    constexpr auto test_env_path = "env path";
    constexpr size_t test_max_db_count{1};

    EXPECT_CALL(api, mdb_env_create(_)).WillOnce(Return(MDB_CORRUPTED));

    auto const env
        = lmdb::make_environment<lmdb::env_flags_t::none, test_max_db_count>(
            test_env_path, lmdb::default_file_mode, api);

    ASSERT_FALSE(env);
    EXPECT_EQ(env.error(), lmdb::error_t::corrupted);
}

TEST(test_create_environment, env_open_failed)
{
    StrictMock<mocks::api> api;

    MDB_env *test_env{reinterpret_cast<MDB_env *>(0x42)};

    constexpr auto test_env_path = "env path";
    constexpr size_t test_max_db_count{1};
    constexpr lmdb::db_file_mode_t test_file_mode{0x42};

    {
        InSequence seq;

        EXPECT_CALL(api, mdb_env_create(_))
            .WillOnce(DoAll(SetArgPointee<0>(test_env), Return(MDB_SUCCESS)));
        EXPECT_CALL(api, mdb_env_set_maxdbs(test_env, test_max_db_count));
        EXPECT_CALL(
            api,
            mdb_env_open(test_env, StrEq(test_env_path), 0, test_file_mode))
            .WillOnce(Return(MDB_BAD_VALSIZE));

        EXPECT_CALL(api, mdb_env_close(test_env));
    }

    auto const env
        = lmdb::make_environment<lmdb::env_flags_t::none, test_max_db_count>(
            test_env_path, test_file_mode, api);

    ASSERT_FALSE(env);
    EXPECT_EQ(env.error(), lmdb::error_t::bad_valsize);
}

}  // namespace cpp_lmdb_tests
