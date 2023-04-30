#pragma once

#include "lmdb.h"

#include <concepts>

namespace lmdb
{
// clang-format off
template <typename Api>
concept lmdb_api_like = requires(Api api) {
    { api.mdb_env_create(std::declval<MDB_env **>()) } -> std::same_as<int>;
    { api.mdb_env_close(std::declval<MDB_env *>()) } -> std::same_as<void>;
    { api.mdb_env_open(
        std::declval<MDB_env *>(), 
        std::declval<char const*>(), 
        std::declval<unsigned int>(), 
        std::declval<mdb_mode_t>()
    ) } -> std::same_as<int>;

    { api.mdb_env_set_maxdbs(
        std::declval<MDB_env *>(),
        std::declval<MDB_dbi>())
    }  -> std::same_as<int>;

    { api.mdb_txn_begin(
        std::declval<MDB_env *>(),
        std::declval<MDB_txn*>(), 
        std::declval<unsigned int>(), 
        std::declval<MDB_txn**>()
    ) } -> std::same_as<int>;
    
    { api.mdb_txn_abort(std::declval<MDB_txn *>()) } -> std::same_as<void>;
    { api.mdb_txn_commit(std::declval<MDB_txn *>()) } -> std::same_as<int>;

    { api.mdb_dbi_open(
        std::declval<MDB_txn *>(), 
        std::declval<char const *>(), 
        std::declval<unsigned int>(), 
        std::declval<MDB_dbi*>()
    ) } -> std::same_as<int>;

    { api.mdb_drop(
        std::declval<MDB_txn *>(), 
        std::declval<MDB_dbi>(),
        std::declval<int>()
    ) } -> std::same_as<int>;

    { api.mdb_set_compare(
        std::declval<MDB_txn *>(), 
        std::declval<MDB_dbi>(),
        std::declval<MDB_cmp_func>()
    ) } -> std::same_as<int>;

    { api.mdb_set_dupsort(
        std::declval<MDB_txn *>(), 
        std::declval<MDB_dbi>(),
        std::declval<MDB_cmp_func>()
    ) } -> std::same_as<int>;

    { api.mdb_put(
        std::declval<MDB_txn *>(), 
        std::declval<MDB_dbi>(),
        std::declval<MDB_val*>(),
        std::declval<MDB_val*>(),
        std::declval<unsigned int>()
    ) } -> std::same_as<int>;

    { api.mdb_get(
        std::declval<MDB_txn *>(), 
        std::declval<MDB_dbi>(),
        std::declval<MDB_val*>(),
        std::declval<MDB_val*>()
    ) } -> std::same_as<int>;

    { api.mdb_del(
        std::declval<MDB_txn *>(), 
        std::declval<MDB_dbi>(),
        std::declval<MDB_val*>(),
        std::declval<MDB_val*>()
    ) } -> std::same_as<int>;

    { api.mdb_cursor_open(
        std::declval<MDB_txn *>(), 
        std::declval<MDB_dbi>(),
        std::declval<MDB_cursor**>()
    ) } -> std::same_as<int>;

    { api.mdb_cursor_close(
        std::declval<MDB_cursor*>()
    ) } -> std::same_as<void>;

    { api.mdb_cursor_get(
        std::declval<MDB_cursor*>(),
        std::declval<MDB_val*>(),
        std::declval<MDB_val*>(),
        std::declval<MDB_cursor_op>()

    ) } -> std::same_as<int>;
};
// clang-format on

}  // namespace lmdb
