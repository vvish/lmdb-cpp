#pragma once

#include "cpp_lmdb/concepts.hpp"
#include "cpp_lmdb/transactions.hpp"
#include "cpp_lmdb/types.hpp"

// details
#include "cpp_lmdb/details/details.hpp"
#include "cpp_lmdb/details/key_value_traits.hpp"

// lmdb
#include "lmdb.h"

namespace lmdb
{
namespace details
{
template <key_value_trait KeyValueTrait, lmdb_api_like LmdbApi>
class db_base {
public:
    db_base(LmdbApi const &api, MDB_dbi const db_index, MDB_env &env)
        : _api{api}, _db_index{db_index}, _env{env}
    {}

protected:
    template <read_only_t ReadOnly>
    using transaction = transaction<KeyValueTrait, ReadOnly, LmdbApi>;

    template <read_only_t ReadOnly>
    auto make_transaction() const LMDB_NOEXCEPT
        -> LMDB_RESULT(transaction<ReadOnly>)
    {
        auto txn = details::make_tx<LmdbApi>(_api, _env, ReadOnly);

        auto const make_transaction
            = [this](auto &&txn) -> LMDB_RESULT(transaction<ReadOnly>) {
            return transaction<ReadOnly>{_db_index, std::move(txn)};
        };

        return LMDB_AND_THEN(std::move(txn), make_transaction);
    }

private:
    LmdbApi const &_api;
    MDB_dbi const _db_index;
    MDB_env &_env;
};
}  // namespace details

template <key_value_trait KeyValueTrait, lmdb_api_like LmdbApi>
class ro_db : public details::db_base<KeyValueTrait, LmdbApi> {
    using base = details::db_base<KeyValueTrait, LmdbApi>;

public:
    using key_type = typename KeyValueTrait::key_trait::value_type;
    using value_type = typename KeyValueTrait::value_trait::value_type;

    using ro_transaction
        = transaction<KeyValueTrait, read_only_t::yes, LmdbApi>;

public:
    using base::base;

    auto begin_ro_transaction() const LMDB_NOEXCEPT
        -> LMDB_RESULT(ro_transaction)
    {
        return base::template make_transaction<read_only_t::yes>();
    }
};

template <key_value_trait KeyValueTrait, lmdb_api_like LmdbApi>
class rw_db : public ro_db<KeyValueTrait, LmdbApi> {
    using base = ro_db<KeyValueTrait, LmdbApi>;

public:
    using rw_transaction
        = transaction<KeyValueTrait, read_only_t::no, LmdbApi>;

public:
    using base::base;

    auto begin_rw_transaction() LMDB_NOEXCEPT -> LMDB_RESULT(rw_transaction)
    {
        return base::template make_transaction<read_only_t::no>();
    }

    auto commit_transaction(rw_transaction &&transaction) LMDB_NOEXCEPT
        -> LMDB_RESULT(void)
    {
        return std::move(transaction).commit();
    }
};

}  // namespace lmdb
