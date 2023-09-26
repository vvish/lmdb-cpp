#pragma once

#include "cpp_lmdb/concepts.hpp"
#include "cpp_lmdb/db_item.hpp"
#include "cpp_lmdb/dbs.hpp"
#include "cpp_lmdb/iterators.hpp"
#include "cpp_lmdb/transactions.hpp"
#include "cpp_lmdb/types.hpp"
#include "cpp_lmdb/views.hpp"

// details
#include "cpp_lmdb/details/api.hpp"
#include "cpp_lmdb/details/details.hpp"
#include "cpp_lmdb/details/key_value_traits.hpp"

// lmdb
#include "lmdb.h"

// std
#include <concepts>
#include <expected>
#include <iterator>
#include <optional>
#include <utility>

namespace lmdb
{

enum class env_flags_t : unsigned int {
    none = 0,
    read_only = MDB_RDONLY,
    no_lock = MDB_NOLOCK,
    //...
};

constexpr auto operator|(env_flags_t lhs, env_flags_t rhs) -> env_flags_t
{
    return env_flags_t{std::to_underlying(lhs) | std::to_underlying(rhs)};
}

constexpr auto operator&(env_flags_t lhs, env_flags_t rhs) -> env_flags_t
{
    return env_flags_t{std::to_underlying(lhs) & std::to_underlying(rhs)};
}

constexpr auto is_readonly(env_flags_t const flags) -> read_only_t
{
    return read_only_t{
        (flags & env_flags_t::read_only) == env_flags_t::read_only};
}

enum class create_if_not_exists { no, yes };

namespace details
{
template <lmdb_api_like LmdbApi>
class environment_base {
public:
    environment_base(details::env_unique_ptr_t<LmdbApi> &&env) noexcept
        : _env{std::move(env)} {};

    environment_base(environment_base &&) = default;
    auto operator=(environment_base &&) -> environment_base & = default;

protected:
    using LmdbApiType = std::remove_reference_t<LmdbApi>;

    template <key_value_trait KeyValueTrait, read_only_t ReadOnly>
    using open_db_result = std::expected<
        std::conditional_t<
            ReadOnly == read_only_t::yes,
            ro_db<KeyValueTrait, LmdbApiType>,
            rw_db<KeyValueTrait, LmdbApiType>>,
        error_t>;

    template <key_value_trait KeyValueTrait, read_only_t ReadOnly>
    auto open_db(
        char const *const name,
        create_if_not_exists const create_flag
        = create_if_not_exists::no) const
        -> open_db_result<KeyValueTrait, ReadOnly>
    {
        auto &api = _env.get_deleter().api;
        auto transaction = make_tx(api, *_env, ReadOnly);
        if (!transaction)
            return std::unexpected{error_t{transaction.error()}};

        const auto creation_flags
            = key_value_trait_helper<KeyValueTrait>::db_key_value_flags()
              | (create_flag == create_if_not_exists::yes ? MDB_CREATE : 0);

        MDB_dbi db_index{};
        LMDB_CALL_API(api.mdb_dbi_open(
            transaction->get(), name, creation_flags, &db_index));

        if constexpr (key_value_trait_helper<KeyValueTrait>::has_key_cmp_fun) {
            LMDB_CALL_API(api.mdb_set_compare(
                transaction->get(),
                db_index,
                cmp<typename KeyValueTrait::key_trait>));
        }

        if constexpr (key_value_trait_helper<
                          KeyValueTrait>::has_value_cmp_fun) {
            LMDB_CALL_API(api.mdb_set_dupsort(
                transaction->get(),
                db_index,
                cmp<typename KeyValueTrait::value_trait>));
        }

        if (auto const result = commit_tx(api, std::move(transaction.value()));
            !result)
            return std::unexpected{error_t{result.error()}};

        return typename open_db_result<KeyValueTrait, ReadOnly>::value_type{
            api, db_index, *_env};
    }

private:
    details::env_unique_ptr_t<LmdbApi> _env;
};

}  // namespace details

template <lmdb_api_like LmdbApi = details::api>
class ro_environment : public details::environment_base<LmdbApi> {
    using base = details::environment_base<LmdbApi>;

public:
    template <key_value_trait KeyValueTrait>
    using ro_db = ro_db<KeyValueTrait, std::remove_reference_t<LmdbApi>>;

    using base::base;

    template <key_value_trait KeyValueTrait>
    auto open_ro_db(char const *const name) const noexcept
        -> std::expected<ro_db<KeyValueTrait>, error_t>
    {
        return details::environment_base<
            LmdbApi>::template open_db<KeyValueTrait, read_only_t::yes>(name);
    }
};

template <lmdb_api_like LmdbApi = details::api>
class rw_environment : public ro_environment<LmdbApi> {
    using base = ro_environment<LmdbApi>;

public:
    template <key_value_trait KeyValueTrait>
    using rw_db = rw_db<KeyValueTrait, std::remove_reference_t<LmdbApi>>;

    using base::base;

    template <key_value_trait KeyValueTrait>
    auto open_rw_db(
        char const *const name,
        create_if_not_exists const create_flag
        = create_if_not_exists::no) noexcept
        -> std::expected<rw_db<KeyValueTrait>, error_t>
    {
        return base::template open_db<KeyValueTrait, read_only_t::no>(
            name, create_flag);
    }
};

template <read_only_t ReadOnly, lmdb_api_like LmdbApi>
using environment_t = std::conditional_t<
    ReadOnly == read_only_t::yes,
    ro_environment<LmdbApi>,
    rw_environment<LmdbApi>>;

using db_file_mode_t = mdb_mode_t;
inline constexpr db_file_mode_t default_file_mode{0600};

template <
    env_flags_t flags,
    size_t max_db_count,
    lmdb_api_like LmdbApi = details::api>
auto make_environment(
    char const *const environment_path,
    db_file_mode_t const db_file_mode,
    LmdbApi &&api = LmdbApi{})
    LMDB_NOEXCEPT->LMDB_RESULT((environment_t<is_readonly(flags), LmdbApi>))
{
    MDB_env *env{nullptr};
    LMDB_CALL_API(api.mdb_env_create(&env));

    std::unique_ptr<MDB_env, details::env_deleter<LmdbApi>> env_ptr{
        env, details::env_deleter<LmdbApi>{std::forward<LmdbApi>(api)}};
    LMDB_CALL_API(api.mdb_env_set_maxdbs(env_ptr.get(), max_db_count));

    LMDB_CALL_API(api.mdb_env_open(
        env_ptr.get(),
        environment_path,
        std::to_underlying(flags),
        db_file_mode));

    return environment_t<is_readonly(flags), LmdbApi>{std::move(env_ptr)};
}

}  // namespace lmdb
