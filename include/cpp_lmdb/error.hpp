#pragma once

#include "lmdb.h"

// std
#ifdef CPP_LMDB_EXCEPTIONS_ENABLED
#include <exception>
#include <functional>
#else
#include <expected>
#endif

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

    auto what() const noexcept -> char const * override
    {
        switch (_error) {
            using enum error_t;
            case key_exist:
                return "Key already exists in the database";
            case not_found:
                return "Key/data pair not found (EOF)";
            case page_not_found:
                return "Requested page not found - this usually indicates "
                       "corruption";
            case corrupted:
                return "Located page was wrong type";
            case panic:
                return "Update of meta page failed or environment had fatal "
                       "error";
            case version_mismatch:
                return "Environment version mismatch";
            case invalid:
                return "File is not a valid LMDB file";
            case map_full:
                return "Environment mapsize reached";
            case dbs_full:
                return "Environment maxdbs reached";
            case readers_full:
                return "Environment maxreaders reached";
            case tls_full:
                return "Thread local storage full";
            case txn_full:
                return "Transaction has too many dirty pages";
            case cursor_full:
                return "Cursor stack too deep - internal error";
            case page_full:
                return "Page has not enough space - internal error";
            case map_resized:
                return "Database contents grew beyond mapsize";
            case incompatible:
                return "The specified database is not compatible with the "
                       "requested operation";
            case bad_rslot:
                return "Invalid reuse of reader locktable slot";
            case bad_txn:
                return "Transaction is not valid for requested operation";
            case bad_valsize:
                return "The specified size is not valid for the specified "
                       "key/data pair";
            case bad_dbi:
                return "The specified DBI handle is not valid for "
                       "requested operation";
        }

        return "Unknown error";
    }

    auto error() const noexcept -> error_t
    {
        return _error;
    }

private:
    error_t _error;
};

#endif  // CPP_LMDB_EXCEPTIONS_ENABLED

}  // namespace lmdb

template <typename T>
struct extract_parantesized_arg;
template <typename T, typename A>
struct extract_parantesized_arg<T(A)> {
    using arg = A;
};
template <typename T>
struct extract_parantesized_arg<T()> {
    using arg = void;
};

#ifdef CPP_LMDB_EXCEPTIONS_ENABLED

struct void_placeholder_t {
    void_placeholder_t() = default;
    void_placeholder_t(void_placeholder_t const &) = default;
    void_placeholder_t(void_placeholder_t &&) = default;
    auto operator=(void_placeholder_t const &)
        -> void_placeholder_t & = default;
    auto operator=(void_placeholder_t &&) -> void_placeholder_t & = default;

    template <typename T, typename... Ts>
        requires(!std::is_same_v<void_placeholder_t, std::decay_t<T>>)
    explicit void_placeholder_t(T &&, Ts &&...) noexcept
    {}
};

template <typename F, typename... Args>
    requires(!std::is_void_v<std::invoke_result_t<F, Args...>>)
auto invoke_void(F &&f, Args &&...args)
{
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

template <typename F, typename... Args>
    requires(std::is_void_v<std::invoke_result_t<F, Args...>>)
auto invoke_void(F &&f, Args &&...args)
{
    std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    return void_placeholder_t{};
}

#define LMDB_RESULT(res_type)                                        \
    typename std::conditional_t<                                     \
        std::is_void_v<                                              \
            typename extract_parantesized_arg<void(res_type)>::arg>, \
        void_placeholder_t,                                          \
        typename extract_parantesized_arg<void(res_type)>::arg>

#define LMDB_NOEXCEPT

#define LMDB_REPORT_ERROR(code)  \
    throw ::lmdb::lmdb_exception \
    {                            \
        code                     \
    }

#define LMDB_REPORT_SUCCESS() \
    return                    \
    {}

#define LMDB_AND_THEN(result, expr)                                     \
    []<typename R, typename E>(R &&r, E &&e) {                          \
        if constexpr (!std::is_same_v<R, void_placeholder_t>) {         \
            return invoke_void(std::forward<E>(e), std::forward<R>(r)); \
        } else {                                                        \
            return invoke_void(std::forward<E>(e));                     \
        }                                                               \
    }(result, expr)

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

#define LMDB_REPORT_SUCCESS() \
    return                    \
    {}

#define LMDB_AND_THEN(result, expr)                             \
    []<typename R, typename E>(R &&r, E &&e) {                  \
        return std::forward<R>(r).and_then(std::forward<E>(e)); \
    }(result, expr)

#endif

#define LMDB_CALL_API(expr)                                    \
    do {                                                       \
        if (auto const result = (expr); result != MDB_SUCCESS) \
            LMDB_REPORT_ERROR(::lmdb::error_t{result});        \
    } while (false)
