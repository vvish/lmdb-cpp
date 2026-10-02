#pragma once

#include "cpp_lmdb/db_item.hpp"
#include "cpp_lmdb/error.hpp"

// details
#include "cpp_lmdb/details/details.hpp"

// std
#include <expected>
#include <iterator>
#include <optional>
#include <vector>

namespace lmdb
{

template <deserialization_trait KeyTrait, deserialization_trait ValueTrait>
class ro_iterator_stub {
public:
    using value_type = ro_db_item<KeyTrait, ValueTrait>;
    using reference = value_type &;
    using const_reference = value_type const &;

    ro_iterator_stub(value_type item) : _item{std::move(item)}
    {}

    auto operator*() const -> const_reference
    {
        return _item;
    }

private:
    value_type _item;
};

namespace details
{
template <
    template <typename, typename, typename>
    class Derived,
    deserialization_trait KeyTrait,
    deserialization_trait ValueTrait,
    lmdb_api_like LmdbApi>
class ro_iterator_base {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = ro_db_item<KeyTrait, ValueTrait>;
    using reference = value_type &;
    using const_reference = value_type const &;
    using difference_type = ptrdiff_t;

public:
    explicit ro_iterator_base(LmdbApi const &api, MDB_cursor &cursor) noexcept
        : _api{api}
        , _cursor{cursor}
        , _item{std::unexpected{error_t::not_found}}
    {}

    ro_iterator_base(ro_iterator_base &&) = default;
    auto operator=(ro_iterator_base &&) -> ro_iterator_base & = default;

    auto operator++(int) -> ro_iterator_stub<KeyTrait, ValueTrait>
    {
        auto const current_item = *_item;

        static_cast<Derived<KeyTrait, ValueTrait, LmdbApi> &>(*this)
            .step_to_next();

        return ro_iterator_stub{current_item};
    };

    auto operator*() const -> const_reference
    {
        return *_item;
    }

    auto operator==(std::default_sentinel_t const &) const -> bool
    {
        return !is_ok();
    }

    auto error() const -> std::optional<error_t>
    {
        return !is_ok() ? std::optional{_item.error()} : std::nullopt;
    }

protected:
    auto navigate_cursor(
        MDB_cursor_op const operation, MDB_val const *const key)
    {
        MDB_val mdb_key = (key != nullptr) ? *key : MDB_val{};
        MDB_val mdb_value{};
        if (auto const result = _api.get().mdb_cursor_get(
                &_cursor.get(), &mdb_key, &mdb_value, operation);
            result != MDB_SUCCESS) {
            _item = std::unexpected{error_t{result}};
        } else {
            _item = ro_db_item<KeyTrait, ValueTrait>{
                to_byte_span(mdb_key), to_byte_span(mdb_value)};
        }
    }

    auto is_ok() const
    {
        return _item.has_value();
    }

private:
    std::reference_wrapper<LmdbApi const> _api;
    std::reference_wrapper<MDB_cursor> _cursor;
    std::expected<value_type, error_t> _item;
};
}  // namespace details

template <key_trait KeyTrait, value_trait ValueTrait, lmdb_api_like LmdbApi>
class ro_iterator
    : public details::
          ro_iterator_base<ro_iterator, KeyTrait, ValueTrait, LmdbApi> {
private:
    using base = details::
        ro_iterator_base<ro_iterator, KeyTrait, ValueTrait, LmdbApi>;

public:
    explicit ro_iterator(LmdbApi const &api, MDB_cursor &cursor) noexcept
        : base{api, cursor}
    {
        base::navigate_cursor(MDB_FIRST, nullptr);
    }

    explicit ro_iterator(
        LmdbApi const &api, MDB_cursor &cursor, byte_span const &key) noexcept
        : base{api, cursor}
    {
        auto db_key = details::to_mdb_val(key);
        base::navigate_cursor(MDB_SET_RANGE, &db_key);
    }

    ro_iterator(ro_iterator &&) = default;
    auto operator=(ro_iterator &&) -> ro_iterator & = default;

    auto operator++() -> ro_iterator &
    {
        step_to_next();
        return *this;
    };

    using base::operator++;

private:
    auto step_to_next()
    {
        base::navigate_cursor(MDB_NEXT, nullptr);
    }
};

template <key_trait KeyTrait, value_trait ValueTrait, lmdb_api_like LmdbApi>
class ro_dup_iterator
    : public details::
          ro_iterator_base<ro_dup_iterator, KeyTrait, ValueTrait, LmdbApi> {
private:
    using base = details::
        ro_iterator_base<ro_dup_iterator, KeyTrait, ValueTrait, LmdbApi>;

public:
    explicit ro_dup_iterator(
        LmdbApi const &api, MDB_cursor &cursor, byte_span const &key) noexcept
        : base{api, cursor}, _key{details::to_mdb_val(key)}
    {
        base::navigate_cursor(MDB_FIRST_DUP, &_key);
    }

    ro_dup_iterator(ro_dup_iterator &&) = default;
    auto operator=(ro_dup_iterator &&) -> ro_dup_iterator & = default;

    auto operator++() -> ro_dup_iterator &
    {
        step_to_next();
        return *this;
    };

    using base::operator++;

private:
    auto step_to_next()
    {
        base::navigate_cursor(MDB_NEXT_DUP, &_key);
    }

    MDB_val _key;
};

}  // namespace lmdb
