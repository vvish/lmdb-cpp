#pragma once

#include "lmdb.h"

namespace lmdb::details
{
#define FORWARD_CALL(method, function)               \
    template <typename... T>                         \
    decltype(auto) method(T &&...params) const       \
    {                                                \
        return function(std::forward<T>(params)...); \
    }

struct api {
    // NOLINTBEGIN(modernize-use-trailing-return-type)
    FORWARD_CALL(mdb_env_create, ::mdb_env_create);
    FORWARD_CALL(mdb_env_close, ::mdb_env_close);
    FORWARD_CALL(mdb_env_open, ::mdb_env_open);

    FORWARD_CALL(mdb_txn_begin, ::mdb_txn_begin);

    FORWARD_CALL(mdb_txn_abort, ::mdb_txn_abort);
    FORWARD_CALL(mdb_txn_commit, ::mdb_txn_commit);
    FORWARD_CALL(mdb_env_set_maxdbs, ::mdb_env_set_maxdbs);

    FORWARD_CALL(mdb_dbi_open, ::mdb_dbi_open);
    FORWARD_CALL(mdb_drop, ::mdb_drop);

    FORWARD_CALL(mdb_set_compare, ::mdb_set_compare);
    FORWARD_CALL(mdb_set_dupsort, ::mdb_set_dupsort);
    FORWARD_CALL(mdb_put, ::mdb_put);
    FORWARD_CALL(mdb_get, ::mdb_get);
    FORWARD_CALL(mdb_del, ::mdb_del);
    FORWARD_CALL(mdb_cursor_open, ::mdb_cursor_open);
    FORWARD_CALL(mdb_cursor_close, ::mdb_cursor_close);
    FORWARD_CALL(mdb_cursor_get, ::mdb_cursor_get);
    // NOLINTEND(modernize-use-trailing-return-type)
};

// static_assert(lmdb::lmdb_api_like<api>);

}  // namespace lmdb::details
