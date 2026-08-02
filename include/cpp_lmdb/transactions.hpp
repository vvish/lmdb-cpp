#pragma once

#include "cpp_lmdb/concepts.hpp"
#include "cpp_lmdb/db_item.hpp"
#include "cpp_lmdb/iterators.hpp"
#include "cpp_lmdb/types.hpp"
#include "cpp_lmdb/views.hpp"

// details
#include "cpp_lmdb/details/details.hpp"
#include "cpp_lmdb/details/key_value_traits.hpp"

// lmdb
#include "lmdb.h"

// std
#include <concepts>

namespace lmdb
{
template <key_value_trait KeyValueTrait, lmdb_api_like LmdbApi>
class rw_db;

template <
    key_value_trait KeyValueTrait,
    read_only_t ReadOnly,
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
        MDB_dbi const db_index,
        details::txn_unique_ptr_t<LmdbApi> &&txn) noexcept
        : _db_index{db_index}
        , _txn{std::move(txn)}
        , _api{_txn.get_deleter().api}
    {}

    auto try_insert(key_type const &key, value_type const &value) LMDB_NOEXCEPT
        -> LMDB_RESULT(void)
        requires(ReadOnly == read_only_t::no)
    {
        return insert_impl(key, value, MDB_NOOVERWRITE);
    }

    auto try_insert_duplicate(key_type const &key, value_type const &value)
        LMDB_NOEXCEPT -> LMDB_RESULT(void)
        requires(
            ReadOnly == read_only_t::no
            && details::key_value_trait_helper<
                KeyValueTrait>::duplicates_enabled)
    {
        return insert_impl(key, value, MDB_NODUPDATA);
    }

    auto insert(key_type const &key, value_type const &value) LMDB_NOEXCEPT
        -> LMDB_RESULT(void)
        requires(ReadOnly == read_only_t::no)
    {
        return insert_impl(key, value, 0);
    }

    auto delete_key(key_type const &key) LMDB_NOEXCEPT -> LMDB_RESULT(void)
    {
        auto const key_bytes = key_trait::to_bytes(key);
        auto mdb_key = to_mdb_val(key_bytes);

        if (auto const result
            = _api.mdb_del(_txn.get(), _db_index, &mdb_key, nullptr);
            result != MDB_SUCCESS) {
            LMDB_REPORT_ERROR(error_t{result});
        }

        LMDB_REPORT_SUCCESS();
    }

    auto get(key_type const &key) const LMDB_NOEXCEPT
        -> LMDB_RESULT(value_type)
        requires(!details::key_value_trait_helper<
                 KeyValueTrait>::duplicates_enabled)
    {
        auto const key_bytes = key_trait::to_bytes(key);
        auto mdb_key = details::to_mdb_val(key_bytes);
        MDB_val mdb_value{};

        if (auto const result
            = _api.mdb_get(_txn.get(), _db_index, &mdb_key, &mdb_value);
            result != MDB_SUCCESS) {
            LMDB_REPORT_ERROR(error_t{result});
        }

        return value_trait::from_bytes(details::to_byte_span(mdb_value));
    }

    // TODO: probably, if exceptions are not enabled, iterator should be
    // returned instead of view as error reporting from views will be limited
    auto iterate() const LMDB_NOEXCEPT -> LMDB_RESULT(ro_view)
    {
        auto cursor_or_error
            = details::make_cursor(_api, _txn.get(), _db_index);

        auto const make_ro_view = [](auto &&cursor) -> LMDB_RESULT(ro_view) {
            return ro_view{std::move(cursor)};
        };

        return LMDB_AND_THEN(std::move(cursor_or_error), make_ro_view);
    }

    auto iterate_by_key(key_type const &key) const LMDB_NOEXCEPT
        -> LMDB_RESULT(ro_dup_view)
        requires(
            details::key_value_trait_helper<KeyValueTrait>::duplicates_enabled)
    {
        auto cursor_or_error
            = details::make_cursor(_api, _txn.get(), _db_index);

        auto const make_ro_dup_view
            = [&key](auto &&cursor) -> LMDB_RESULT(ro_dup_view) {
            auto const key_bytes = key_trait::to_bytes(key);
            return ro_dup_view{std::move(cursor), key_bytes};
        };

        return LMDB_AND_THEN(std::move(cursor_or_error), make_ro_dup_view);
    }

private:
    auto insert_impl(
        key_type const &key, value_type const &value, unsigned int flags)
        -> LMDB_RESULT(void)
    {
        const auto key_bytes = key_trait::to_bytes(key);
        auto mdb_key = details::to_mdb_val(key_bytes);
        const auto value_bytes = value_trait::to_bytes(value);
        auto mdb_value = details::to_mdb_val(value_bytes);

        LMDB_CALL_API(
            _api.mdb_put(_txn.get(), _db_index, &mdb_key, &mdb_value, flags));

        LMDB_REPORT_SUCCESS();
    }

    auto commit() && LMDB_NOEXCEPT -> LMDB_RESULT(void)
    {
        return commit_tx(_api, std::move(_txn));
    }

private:
    MDB_dbi const _db_index;
    details::txn_unique_ptr_t<LmdbApi> _txn;
    LmdbApi const &_api;
};

}  // namespace lmdb
