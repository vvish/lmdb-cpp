#pragma once

#include "cpp_lmdb/types.hpp"

// lmdb
#include "lmdb.h"

// std
#include <expected>
#include <memory>
#include <span>
#include <type_traits>

namespace lmdb::details
{
template <typename LmdbApi>
struct env_deleter {
    auto operator()(MDB_env *env) noexcept -> void
    {
        api.mdb_env_close(env);
    }

    LmdbApi api;
};

template <typename LmdbApi>
using env_unique_ptr_t = std::unique_ptr<MDB_env, env_deleter<LmdbApi>>;

inline auto to_byte_span(MDB_val const &value) noexcept
{
    return std::span{
        static_cast<std::byte const *>(value.mv_data), value.mv_size};
}

inline auto to_mdb_val(byte_span const &value) noexcept
{
    return MDB_val{value.size(), const_cast<std::byte *>(value.data())};
}

template <typename Trait>
auto cmp(MDB_val const *lhs, MDB_val const *rhs) -> int
{
    return Trait::cmp(to_byte_span(*lhs), to_byte_span(*rhs));
}

template <typename LmdbApi>
struct txn_deleter {
    auto operator()(MDB_txn *txn) noexcept -> void
    {
        api.mdb_txn_abort(txn);
    }

    LmdbApi const &api;
};

template <typename LmdbApi>
using txn_unique_ptr_t = std::unique_ptr<MDB_txn, txn_deleter<LmdbApi>>;

template <typename LmdbApi>
constexpr auto make_tx(
    LmdbApi const &api, MDB_env &env, read_only_t const read_only)
    -> std::expected<txn_unique_ptr_t<LmdbApi>, int>
{
    MDB_txn *txn{nullptr};
    if (auto const result = api.mdb_txn_begin(
            &env,
            nullptr,
            read_only == read_only_t::yes ? MDB_RDONLY : 0,
            &txn);
        result != MDB_SUCCESS) {
        return std::unexpected{result};
    }

    return txn_unique_ptr_t{txn, details::txn_deleter<LmdbApi>{api}};
}

template <typename LmdbApi>
constexpr auto commit_tx(LmdbApi const &api, txn_unique_ptr_t<LmdbApi> &&tx)
    -> std::expected<void, int>
{
    if (auto const result = api.mdb_txn_commit(tx.release());
        result != MDB_SUCCESS) {
        return std::unexpected{result};
    }

    return {};
}

template <typename LmdbApi>
struct cursor_deleter {
    auto operator()(MDB_cursor *cursor) noexcept -> void
    {
        api.get().mdb_cursor_close(cursor);
    }

    std::reference_wrapper<LmdbApi const> api;
};

template <typename LmdbApi>
using cursor_unique_ptr_t
    = std::unique_ptr<MDB_cursor, cursor_deleter<LmdbApi>>;

template <typename LmdbApi>
constexpr auto make_cursor(
    LmdbApi const &api, MDB_txn *const txn, MDB_dbi const dbi)
    -> std::expected<cursor_unique_ptr_t<LmdbApi>, int>
{
    MDB_cursor *cursor{nullptr};
    if (auto const result = api.mdb_cursor_open(txn, dbi, &cursor);
        result != MDB_SUCCESS) {
        return std::unexpected{result};
    }

    return cursor_unique_ptr_t{cursor, cursor_deleter<LmdbApi>{api}};
}

}  // namespace lmdb::details
