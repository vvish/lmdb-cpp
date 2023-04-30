#pragma once

#include "lmdb.h"

// std
#include <exception>
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

#ifdef CPP_LMDB_EXCEPTIONS_ENABLED

class lmdb_exception : public std::exception {
public:
    lmdb_exception(error_t error) noexcept : _error{error}
    {}

    auto what() const noexcept -> char const* override
    {
        return "";
    }

    auto error() const noexcept -> error_t
    {
        return _error;
    }

private:
    error_t _error;
};

#endif // CPP_LMDB_EXCEPTIONS_ENABLED

}  // namespace lmdb

template <typename T>
struct extract_parantesized_arg;
template <typename T, typename A>
struct extract_parantesized_arg<T(A)> {
    using arg = A;
};

#ifdef CPP_LMDB_EXCEPTIONS_ENABLED

#define LMDB_RESULT(res_type) \
    typename extract_parantesized_arg<void(res_type)>::arg
#define LMDB_NOEXCEPT
// #define LMDB_NOEXCEPT_COND(cond) noexcept(cond)

#define LMDB_REPORT_ERROR(code)  \
    throw ::lmdb::lmdb_exception \
    {                            \
        code                     \
    }

#else

#define LMDB_RESULT(res_type)                                   \
    std::expected<                                              \
        typename extract_parantesized_arg<void(res_type)>::arg, \
        ::lmdb::error_t>

#define LMDB_NOEXCEPT noexcept

#define LMDB_REPORT_ERROR(code) \
    return std::unexpected      \
    {                           \
        code                    \
    }

#endif

#define LMDB_CALL_API(expr)                                    \
    do {                                                       \
        if (auto const result = (expr); result != MDB_SUCCESS) \
            LMDB_REPORT_ERROR(::lmdb::error_t{result});        \
    } while (false)
