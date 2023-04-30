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

using test_trait
    = lmdb::unique_key<lmdb::trivial_trait<int>, lmdb::trivial_trait<int>>;

class test_create_db : public Test {
protected:
    StrictMock<mocks::api> api;

    MDB_env *test_env{reinterpret_cast<MDB_env *>(0x42)};
    lmdb::details::env_unique_ptr_t<StrictMock<mocks::api> &> env{
        test_env, lmdb::details::env_deleter<StrictMock<mocks::api> &>{api}};

    MDB_txn *test_tx{reinterpret_cast<MDB_txn *>(0x84)};

    constexpr static MDB_dbi test_dbi{10};
    constexpr static auto test_db_name{"db name"};
};

template <typename T>
constexpr bool db_has_begin_rw_transaction_v
    = requires(T t) { t.begin_rw_transaction(); };

TEST_F(test_create_db, open_rw_db_trivial_values)
{
    lmdb::rw_environment<StrictMock<mocks::api> &> environment{std::move(env)};

    {
        InSequence seq;

        EXPECT_CALL(api, mdb_txn_begin(test_env, nullptr, 0, _))
            .WillOnce(DoAll(SetArgPointee<3>(test_tx), Return(MDB_SUCCESS)));
        EXPECT_CALL(api, mdb_dbi_open(test_tx, StrEq(test_db_name), 0, _))
            .WillOnce(DoAll(SetArgPointee<3>(test_dbi), Return(MDB_SUCCESS)));
        EXPECT_CALL(api, mdb_txn_commit(test_tx))
            .WillOnce(Return(MDB_SUCCESS));
        EXPECT_CALL(api, mdb_env_close(test_env));
    }

    auto db = environment.open_rw_db<
        lmdb::unique_key<lmdb::trivial_trait<int>, lmdb::trivial_trait<int>>>(
        test_db_name);

    ASSERT_TRUE(db);

    static_assert(db_has_begin_rw_transaction_v<decltype(db.value())>);
}

TEST_F(test_create_db, open_rw_data_base_create_if_not_existent_trivial_values)
{
    lmdb::rw_environment<StrictMock<mocks::api> &> environment{std::move(env)};

    {
        InSequence seq;

        EXPECT_CALL(api, mdb_txn_begin(test_env, nullptr, 0, _))
            .WillOnce(DoAll(SetArgPointee<3>(test_tx), Return(MDB_SUCCESS)));
        EXPECT_CALL(
            api, mdb_dbi_open(test_tx, StrEq(test_db_name), MDB_CREATE, _))
            .WillOnce(DoAll(SetArgPointee<3>(test_dbi), Return(MDB_SUCCESS)));
        EXPECT_CALL(api, mdb_txn_commit(test_tx))
            .WillOnce(Return(MDB_SUCCESS));
        EXPECT_CALL(api, mdb_env_close(test_env));
    }

    auto const db = environment.open_rw_db<
        lmdb::unique_key<lmdb::trivial_trait<int>, lmdb::trivial_trait<int>>>(
        test_db_name, lmdb::create_if_not_exists::yes);
}

struct test_duplicate_reversed_key_value_trait_helper {
    using duplicate_key_flag = std::true_type;

    struct key_trait : lmdb::trivial_trait<int> {
        using reversed_flag = std::true_type;
    };

    struct value_trait : lmdb::trivial_trait<int> {
        using reversed_flag = std::true_type;
    };
};

TEST_F(test_create_db, open_rw_data_base_custom_order)
{
    lmdb::rw_environment<StrictMock<mocks::api> &> environment{std::move(env)};

    {
        InSequence seq;

        EXPECT_CALL(api, mdb_txn_begin(test_env, nullptr, 0, _))
            .WillOnce(DoAll(SetArgPointee<3>(test_tx), Return(MDB_SUCCESS)));
        EXPECT_CALL(
            api,
            mdb_dbi_open(
                test_tx,
                StrEq(test_db_name),
                MDB_DUPSORT | MDB_REVERSEKEY | MDB_REVERSEDUP,
                _))
            .WillOnce(DoAll(SetArgPointee<3>(test_dbi), Return(MDB_SUCCESS)));
        EXPECT_CALL(api, mdb_txn_commit(test_tx))
            .WillOnce(Return(MDB_SUCCESS));
        EXPECT_CALL(api, mdb_env_close(test_env));
    }

    auto db = environment
                  .open_rw_db<test_duplicate_reversed_key_value_trait_helper>(
                      test_db_name);

    ASSERT_TRUE(db);

    static_assert(db_has_begin_rw_transaction_v<decltype(db.value())>);
}

struct test_integer_dup_key_integer_value_trait {
    using duplicate_key_flag = std::true_type;

    using key_trait = lmdb::trivial_trait<unsigned int>;
    using value_trait = lmdb::trivial_trait<size_t>;
};

TEST_F(test_create_db, open_rw_db_integer_key)
{
    lmdb::rw_environment<StrictMock<mocks::api> &> environment{std::move(env)};
    {
        InSequence seq;

        EXPECT_CALL(api, mdb_txn_begin(test_env, nullptr, 0, _))
            .WillOnce(DoAll(SetArgPointee<3>(test_tx), Return(MDB_SUCCESS)));
        EXPECT_CALL(
            api,
            mdb_dbi_open(
                test_tx,
                StrEq(test_db_name),
                MDB_DUPSORT | MDB_INTEGERKEY | MDB_INTEGERDUP,
                _))
            .WillOnce(DoAll(SetArgPointee<3>(test_dbi), Return(MDB_SUCCESS)));
        EXPECT_CALL(api, mdb_txn_commit(test_tx))
            .WillOnce(Return(MDB_SUCCESS));
        EXPECT_CALL(api, mdb_env_close(test_env));
    }

    auto db = environment.open_rw_db<test_integer_dup_key_integer_value_trait>(
        test_db_name);

    ASSERT_TRUE(db);

    static_assert(db_has_begin_rw_transaction_v<decltype(db.value())>);
}

struct test_custom_order_fn_key_value_trait_helper {
    using duplicate_key_flag = std::true_type;

    struct key_trait : lmdb::trivial_trait<int> {
        static auto cmp(lmdb::byte_span const &, lmdb::byte_span const &)
            -> int
        {
            return {};
        }
    };

    struct value_trait : lmdb::trivial_trait<int> {
        static auto cmp(lmdb::byte_span const &, lmdb::byte_span const &)
            -> int
        {
            return {};
        }
    };
};

TEST_F(test_create_db, open_rw_db_custom_cmp_fn)
{
    lmdb::rw_environment<StrictMock<mocks::api> &> environment{std::move(env)};
    {
        InSequence seq;

        EXPECT_CALL(api, mdb_txn_begin(test_env, nullptr, 0, _))
            .WillOnce(DoAll(SetArgPointee<3>(test_tx), Return(MDB_SUCCESS)));
        EXPECT_CALL(
            api, mdb_dbi_open(test_tx, StrEq(test_db_name), MDB_DUPSORT, _))
            .WillOnce(DoAll(SetArgPointee<3>(test_dbi), Return(MDB_SUCCESS)));
        EXPECT_CALL(api, mdb_set_compare(test_tx, test_dbi, _))
            .WillOnce(Return(MDB_SUCCESS));
        EXPECT_CALL(api, mdb_set_dupsort(test_tx, test_dbi, _))
            .WillOnce(Return(MDB_SUCCESS));
        EXPECT_CALL(api, mdb_txn_commit(test_tx))
            .WillOnce(Return(MDB_SUCCESS));
        EXPECT_CALL(api, mdb_env_close(test_env));
    }

    auto db
        = environment.open_rw_db<test_custom_order_fn_key_value_trait_helper>(
            test_db_name);
}
}  // namespace cpp_lmdb_tests
