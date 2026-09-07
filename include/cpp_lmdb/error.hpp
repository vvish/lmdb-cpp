#pragma once

#include "lmdb.h"

// std
#include <expected>

namespace lmdb
{

enum class error_t : int {
    key_exist = MDB_KEYEXIST,
    not_found = MDB_NOTFOUND,
    page_not_found = MDB_PAGE_NOTFOUND,
    corrupted = MDB_CORRUPTED,
    panic = MDB_PANIC,
    version_mismatch = MDB_VERSION_MISMATCH,
    invalid = MDB_INVALID,
    map_full = MDB_MAP_FULL,
    dbs_full = MDB_DBS_FULL,
    readers_full = MDB_READERS_FULL,
    tls_full = MDB_TLS_FULL,
    txn_full = MDB_TXN_FULL,
    cursor_full = MDB_CURSOR_FULL,
    page_full = MDB_PAGE_FULL,
    map_resized = MDB_MAP_RESIZED,
    incompatible = MDB_INCOMPATIBLE,
    bad_rslot = MDB_BAD_RSLOT,
    bad_txn = MDB_BAD_TXN,
    bad_valsize = MDB_BAD_VALSIZE,
    bad_dbi = MDB_BAD_DBI,
};

}  // namespace lmdb

#define LMDB_CALL_API(expr)                                    \
    do {                                                       \
        if (auto const result = (expr); result != MDB_SUCCESS) \
            return std::unexpected{::lmdb::error_t{result}};   \
    } while (false)
