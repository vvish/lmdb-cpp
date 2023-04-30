#pragma once

#include "cpp_lmdb/cpp_lmdb.hpp"

// gtest
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <concepts>
using namespace ::testing;  // NOLINT(google-build-using-namespace)

namespace cpp_lmdb_tests::mocks
{
struct api {
    // NOLINTBEGIN(modernize-use-trailing-return-type)
    MOCK_METHOD(int, mdb_env_create, (MDB_env **), (const));
    MOCK_METHOD(void, mdb_env_close, (MDB_env *), (const));
    MOCK_METHOD(
        int,
        mdb_env_open,
        (MDB_env *, char const *, unsigned int, mdb_mode_t),
        (const));
    MOCK_METHOD(
        int,
        mdb_txn_begin,
        (MDB_env *, MDB_txn *, unsigned int, MDB_txn **),
        (const));
    MOCK_METHOD(void, mdb_txn_abort, (MDB_txn *), (const));
    MOCK_METHOD(int, mdb_txn_commit, (MDB_txn *), (const));
    MOCK_METHOD(int, mdb_env_set_maxdbs, (MDB_env *, MDB_dbi), (const));
    MOCK_METHOD(
        int,
        mdb_dbi_open,
        (MDB_txn *, char const *, unsigned int, MDB_dbi *),
        (const));

    MOCK_METHOD(int, mdb_drop, (MDB_txn *, MDB_dbi, int), (const));

    MOCK_METHOD(
        int, mdb_set_compare, (MDB_txn *, MDB_dbi, MDB_cmp_func), (const));
    MOCK_METHOD(
        int, mdb_set_dupsort, (MDB_txn *, MDB_dbi, MDB_cmp_func), (const));
    MOCK_METHOD(
        int,
        mdb_put,
        (MDB_txn *, MDB_dbi, MDB_val *, MDB_val *, unsigned int),
        (const));
    MOCK_METHOD(
        int, mdb_get, (MDB_txn *, MDB_dbi, MDB_val *, MDB_val *), (const));
    MOCK_METHOD(
        int, mdb_del, (MDB_txn *, MDB_dbi, MDB_val *, MDB_val *), (const));
    MOCK_METHOD(
        int, mdb_cursor_open, (MDB_txn *, MDB_dbi, MDB_cursor **), (const));
    MOCK_METHOD(void, mdb_cursor_close, (MDB_cursor *), (const));
    MOCK_METHOD(
        int,
        mdb_cursor_get,
        (MDB_cursor *, MDB_val *, MDB_val *, MDB_cursor_op),
        (const));

    // NOLINTEND(modernize-use-trailing-return-type)
};

static_assert(lmdb::lmdb_api_like<api>);

}  // namespace cpp_lmdb_tests::mocks
