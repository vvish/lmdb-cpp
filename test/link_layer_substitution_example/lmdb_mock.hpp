#pragma once

// lmdb
#include "lmdb.h"

// gtest
#include "gmock/gmock.h"

namespace test
{
struct lmdb_mock {
    // NOLINTBEGIN(modernize-use-trailing-return-type)
    MOCK_METHOD(int, mdb_env_create, (MDB_env **), (const));
    MOCK_METHOD(
        int,
        mdb_env_open,
        (MDB_env *, char const *, unsigned int, mdb_mode_t),
        (const));
    // NOLINTEND(modernize-use-trailing-return-type)
};

void register_lmdb_mock(lmdb_mock &mock);
void unregister_lmdb_mock();

}  // namespace test
