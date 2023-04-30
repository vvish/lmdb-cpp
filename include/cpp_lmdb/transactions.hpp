#pragma once

#include "cpp_lmdb/concepts.hpp"
#include "cpp_lmdb/db_item.hpp"
#include "cpp_lmdb/iterators.hpp"
#include "cpp_lmdb/views.hpp"
#include "cpp_lmdb/types.hpp"

// details
#include "cpp_lmdb/details/details.hpp"
#include "cpp_lmdb/details/key_value_traits.hpp"

// lmdb
#include "lmdb.h"

// std
#include <concepts>
#include <expected>

namespace lmdb
{
template <key_value_trait KeyValueTrait, lmdb_api_like LmdbApi>
class rw_db;

template <
    key_value_trait KeyValueTrait,
    read_only_t read_only,
    lmdb_api_like LmdbApi>
class transaction {
    friend class rw_db<KeyValueTrait, LmdbApi>;

public:
    using key_trait = typename KeyValueTrait::key_trait;
    using value_trait = typename KeyValueTrait::value_trait;

    using key_type = key_trait::value_type;
    using value_type = value_trait::value_type;

    using ro_view
        = db_view<ro_iterator<key_trait, value_trait, LmdbApi>, LmdbApi>;
    using ro_dup_view = db_dup_view<
        ro_dup_iterator<key_trait, value_trait, LmdbApi>,
        LmdbApi>;

public:
    transaction(
        MDB_dbi const db_index, details::txn_unique_ptr_t<LmdbApi> &&txn) noexcept
        : _db_index{db_index}
        , _txn{std::move(txn)}
        , _api{_txn.get_deleter().api}
    {}

    auto try_insert(key_type const &key, value_type const &value) noexcept
        -> std::expected<void, error_t>
        requires(read_only == read_only_t::no)
    {
        return insert_impl(key, value, MDB_NOOVERWRITE);
    }

    auto try_insert_duplicate(
        key_type const &key, value_type const &value) noexcept
        -> std::expected<void, error_t>
        requires(
            read_only == read_only_t::no
            && details::key_value_trait_helper<
                KeyValueTrait>::duplicates_enabled)
    {
        return insert_impl(key, value, MDB_NODUPDATA);
    }

    auto insert(key_type const &key, value_type const &value) noexcept
        -> std::expected<void, error_t>
        requires(read_only == read_only_t::no)
    {
        return insert_impl(key, value, 0);
    }

    auto delete_key(key_type const &key) noexcept
        -> std::expected<void, error_t>
    {
        auto const key_bytes = key_trait::to_bytes(key);
        auto mdb_key = to_mdb_val(key_bytes);

        if (auto const result
            = _api.mdb_del(_txn.get(), _db_index, &mdb_key, nullptr);
            result != MDB_SUCCESS) {
            return std::unexpected{error_t{result}};
        }

        return {};
    }

    auto get(key_type const &key) const noexcept
        -> std::expected<value_type, error_t>
        requires(!details::key_value_trait_helper<
                 KeyValueTrait>::duplicates_enabled)
    {
        auto const key_bytes = key_trait::to_bytes(key);
        auto mdb_key = details::to_mdb_val(key_bytes);
        MDB_val mdb_value{};

        if (auto const result
            = _api.mdb_get(_txn.get(), _db_index, &mdb_key, &mdb_value);
            result != MDB_SUCCESS) {
            return std::unexpected{error_t{result}};
        }

        return value_trait::from_bytes(details::to_byte_span(mdb_value));
    }

    // TODO: probably, if exceptions are not enabled, iterator should be
    // returned instead of view as error reporting from views will be limited
    auto iterate() const noexcept -> std::expected<ro_view, error_t>
    {
        auto cursor = details::make_cursor(_api, _txn.get(), _db_index);
        if (!cursor)
            return std::unexpected{error_t{cursor.error()}};

        return ro_view{std::move(*cursor)};
    }

    auto iterate_by_key(key_type const &key) const noexcept
        -> std::expected<ro_dup_view, error_t>
        requires(
            details::key_value_trait_helper<KeyValueTrait>::duplicates_enabled)
    {
        auto cursor = make_cursor(_api, _txn.get(), _db_index);
        if (!cursor)
            return std::unexpected{error_t{cursor.error()}};

        auto const key_bytes = key_trait::to_bytes(key);
        auto mdb_key = to_mdb_val(key_bytes);

        return ro_dup_view{std::move(*cursor), key_bytes};
    }

private:
    auto insert_impl(
        key_type const &key, value_type const &value, unsigned int flags)
        -> std::expected<void, error_t>
    {
        const auto key_bytes = key_trait::to_bytes(key);
        auto mdb_key = details::to_mdb_val(key_bytes);
        const auto value_bytes = value_trait::to_bytes(value);
        auto mdb_value = details::to_mdb_val(value_bytes);

        if (auto const result
            = _api.mdb_put(_txn.get(), _db_index, &mdb_key, &mdb_value, flags);
            result != MDB_SUCCESS) {
            return std::unexpected{error_t{result}};
        }

        return {};
    }

    auto commit() && noexcept -> std::expected<void, error_t>
    {
        if (auto const result = commit_tx(_api, std::move(_txn)); !result) {
            return std::unexpected{error_t{result.error()}};
        }

        return {};
    }

private:
    MDB_dbi const _db_index;
    details::txn_unique_ptr_t<LmdbApi> _txn;
    LmdbApi const &_api;
};

}  // namespace lmdb
