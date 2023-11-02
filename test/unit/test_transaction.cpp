#include "cpp_lmdb/cpp_lmdb.hpp"
#include "mocks.hpp"

// gtest
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <concepts>

namespace cpp_lmdb_tests
{
using namespace ::testing;  // NOLINT(google-build-using-namespace)

class test_transaction : public Test {
protected:
    StrictMock<mocks::api> api;

    MDB_txn *test_txn{reinterpret_cast<MDB_txn *>(0x42)};
    lmdb::details::txn_unique_ptr_t<StrictMock<mocks::api>> txn{
        test_txn, lmdb::details::txn_deleter<StrictMock<mocks::api>>{api}};

    constexpr static MDB_dbi test_dbi{10};
};

namespace
{
class MdbValBytesAre {
public:
    using is_gtest_matcher = void;

    MdbValBytesAre(std::initializer_list<uint8_t> byte_span)
        : _bytes{byte_span}
    {}

    auto MatchAndExplain(const MDB_val &val, std::ostream *os) const
    {
        if (val.mv_size != _bytes.size()) {
            if (os) {
                *os << "Mdb_val size (" << val.mv_size
                    << ") doesn't match expected size (" << _bytes.size()
                    << ")";
            }
            return false;
        }

        auto const *const arg_bytes
            = static_cast<uint8_t const *>(val.mv_data);
        for (size_t i = 0; i < val.mv_size; ++i) {
            if (_bytes[i] != arg_bytes[i])
                return false;
        }

        return true;
    }

    auto DescribeTo(std::ostream *os) const
    {
        *os << "MDB_val content matches";
    }

    auto DescribeNegationTo(std::ostream *os) const
    {
        *os << "MDB_val content doesn't match";
    }

private:
    std::vector<uint8_t> _bytes;
};
}  // namespace

using test_trait
    = lmdb::unique_key<lmdb::trivial_trait<int>, lmdb::trivial_trait<int>>;

TEST_F(test_transaction, transaction_implicit_abort)
{
    lmdb::
        transaction<test_trait, lmdb::read_only_t::no, StrictMock<mocks::api>>
            transaction{test_dbi, std::move(txn)};

    EXPECT_CALL(api, mdb_txn_abort(test_txn));
}

TEST_F(test_transaction, trivial_types_transaction_insert)
{
    lmdb::
        transaction<test_trait, lmdb::read_only_t::no, StrictMock<mocks::api>>
            transaction{test_dbi, std::move(txn)};
    {
        InSequence const seq;

        EXPECT_CALL(
            api,
            mdb_put(
                test_txn,
                test_dbi,
                Pointee(MdbValBytesAre{0x78, 0x56, 0x34, 0x12}),
                Pointee(MdbValBytesAre{0x30, 0x0, 0x0, 0x20}),
                0))
            .WillOnce(Return(MDB_SUCCESS));

        EXPECT_CALL(
            api,
            mdb_put(
                test_txn,
                test_dbi,
                Pointee(MdbValBytesAre{0x78, 0x56, 0x34, 0x12}),
                Pointee(MdbValBytesAre{0x30, 0x0, 0x0, 0x20}),
                0))
            .WillOnce(Return(MDB_BAD_TXN));

        EXPECT_CALL(api, mdb_txn_abort(test_txn));
    }

    EXPECT_TRUE(transaction.insert(0x12345678, 0x20000030));
    auto const result = transaction.insert(0x12345678, 0x20000030);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), lmdb::error_t::bad_txn);
}

TEST_F(test_transaction, trivial_types_transaction_try_insert)
{
    lmdb::
        transaction<test_trait, lmdb::read_only_t::no, StrictMock<mocks::api>>
            transaction{test_dbi, std::move(txn)};
    {
        InSequence const seq;

        EXPECT_CALL(
            api,
            mdb_put(
                test_txn,
                test_dbi,
                Pointee(MdbValBytesAre{0x78, 0x56, 0x34, 0x12}),
                Pointee(MdbValBytesAre{0x30, 0x0, 0x0, 0x20}),
                MDB_NOOVERWRITE))
            .WillOnce(Return(MDB_SUCCESS));

        EXPECT_CALL(
            api,
            mdb_put(
                test_txn,
                test_dbi,
                Pointee(MdbValBytesAre{0x78, 0x56, 0x34, 0x12}),
                Pointee(MdbValBytesAre{0x30, 0x0, 0x0, 0x20}),
                MDB_NOOVERWRITE))
            .WillOnce(Return(MDB_KEYEXIST));

        EXPECT_CALL(api, mdb_txn_abort(test_txn));
    }

    EXPECT_TRUE(transaction.try_insert(0x12345678, 0x20000030));
    auto const result = transaction.try_insert(0x12345678, 0x20000030);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), lmdb::error_t::key_exist);
}

TEST_F(test_transaction, trivial_types_transaction_get)
{
    std::array<uint8_t, 4> test_value{0x30, 0x0, 0x0, 0x20};

    lmdb::
        transaction<test_trait, lmdb::read_only_t::no, StrictMock<mocks::api>>
            transaction{test_dbi, std::move(txn)};

    {
        InSequence const seq;

        EXPECT_CALL(
            api,
            mdb_get(
                test_txn,
                test_dbi,
                Pointee(MdbValBytesAre{0x78, 0x56, 0x34, 0x12}),
                _))
            .WillOnce(DoAll(
                SetArgPointee<3>(
                    MDB_val{test_value.size(), test_value.data()}),
                Return(MDB_SUCCESS)));

        EXPECT_CALL(api, mdb_txn_abort(test_txn));
    }

    const auto result = transaction.get(0x12345678);
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 0x20000030);
}

TEST_F(test_transaction, trivial_types_transaction_get_not_found)
{
    lmdb::
        transaction<test_trait, lmdb::read_only_t::no, StrictMock<mocks::api>>
            transaction{test_dbi, std::move(txn)};

    {
        InSequence const seq;

        EXPECT_CALL(
            api,
            mdb_get(
                test_txn,
                test_dbi,
                Pointee(MdbValBytesAre{0x78, 0x56, 0x34, 0x12}),
                _))
            .WillOnce(Return(MDB_NOTFOUND));

        EXPECT_CALL(api, mdb_txn_abort(test_txn));
    }

    const auto result = transaction.get(0x12345678);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), lmdb::error_t::not_found);
}

template <typename T>
concept env_has_iterate_by_key_v
    = requires(T t) { t.template iterate_by_key(std::declval<int>()); };

TEST_F(test_transaction, trivial_types_transaction_iterate)
{
    lmdb::
        transaction<test_trait, lmdb::read_only_t::no, StrictMock<mocks::api>>
            transaction{test_dbi, std::move(txn)};

    static_assert(!env_has_iterate_by_key_v<decltype(transaction)>);

    auto *cursor{reinterpret_cast<MDB_cursor *>(0x84)};

    {
        InSequence const seq;

        EXPECT_CALL(api, mdb_cursor_open(test_txn, test_dbi, _))
            .WillOnce(DoAll(SetArgPointee<2>(cursor), Return(MDB_SUCCESS)));

        EXPECT_CALL(api, mdb_cursor_close(cursor));
        EXPECT_CALL(api, mdb_txn_abort(test_txn));
    }

    const auto result = transaction.iterate();
    ASSERT_TRUE(result);
}

using test_trait_dup
    = lmdb::duplicate_key<lmdb::trivial_trait<int>, lmdb::trivial_trait<int>>;

TEST_F(test_transaction, trivial_types_dup_iterate_by_key)
{
    lmdb::transaction<
        test_trait_dup,
        lmdb::read_only_t::no,
        StrictMock<mocks::api>>
        transaction{test_dbi, std::move(txn)};

    auto *cursor{reinterpret_cast<MDB_cursor *>(0x84)};

    {
        InSequence const seq;

        EXPECT_CALL(api, mdb_cursor_open(test_txn, test_dbi, _))
            .WillOnce(DoAll(SetArgPointee<2>(cursor), Return(MDB_SUCCESS)));

        EXPECT_CALL(api, mdb_cursor_close(cursor));
        EXPECT_CALL(api, mdb_txn_abort(test_txn));
    }

    const auto result = transaction.iterate();
    ASSERT_TRUE(result);
}

TEST_F(test_transaction, trivial_types_transaction_lower_bound)
{
    std::array<uint8_t, 4> test_value{0x30, 0x0, 0x0, 0x20};

    lmdb::
        transaction<test_trait, lmdb::read_only_t::no, StrictMock<mocks::api>>
            transaction{test_dbi, std::move(txn)};

    auto *cursor{reinterpret_cast<MDB_cursor *>(0x84)};

    {
        InSequence const seq;

        EXPECT_CALL(api, mdb_cursor_open(test_txn, test_dbi, _))
            .WillOnce(DoAll(SetArgPointee<2>(cursor), Return(MDB_SUCCESS)));

        EXPECT_CALL(api, mdb_cursor_get(cursor, Pointee(MdbValBytesAre{0x78, 0x56, 0x34, 0x12}), _, MDB_SET_RANGE))
            .WillOnce(DoAll(
                SetArgPointee<2>(
                    MDB_val{test_value.size(), test_value.data()}),
                Return(MDB_SUCCESS)));

        EXPECT_CALL(api, mdb_cursor_close(cursor));
        EXPECT_CALL(api, mdb_txn_abort(test_txn));
    }

    const auto result = transaction.lower_bound(0x12345678);
    ASSERT_TRUE(result);

    auto const& db_view = *result;
    auto const it = db_view.begin();
    EXPECT_NE(it, db_view.end());

    auto const& key_value = *it;
    EXPECT_EQ(key_value.key(), 0x12345678);
    EXPECT_EQ(key_value.value(), 0x20000030);
}

}  // namespace cpp_lmdb_tests
