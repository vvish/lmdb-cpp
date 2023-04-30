#pragma once

#include "cpp_lmdb/iterators.hpp"

#include <ranges>

namespace lmdb
{

template <std::input_iterator Iterator, lmdb_api_like LmdbApi>
class db_view
    : public std::ranges::view_interface<db_view<Iterator, LmdbApi>> {
public:
    explicit db_view(details::cursor_unique_ptr_t<LmdbApi> &&cursor) noexcept
        : _cursor{std::move(cursor)}
    {}

    db_view(db_view &&) noexcept = default;
    auto operator=(db_view &&) noexcept -> db_view & = default;

    auto begin() const noexcept
    {
        auto const &api = _cursor.get_deleter().api;
        return Iterator{api.get(), *_cursor};
    }

    auto end() const noexcept
    {
        return std::default_sentinel;
    }

private:
    details::cursor_unique_ptr_t<LmdbApi> _cursor;
};

template <std::input_iterator Iterator, lmdb_api_like LmdbApi>
class db_dup_view
    : public std::ranges::view_interface<db_dup_view<Iterator, LmdbApi>> {
public:
    explicit db_dup_view(
        details::cursor_unique_ptr_t<LmdbApi> &&cursor, byte_span const &key)
        : _cursor{std::move(cursor)}, _key{key.begin(), key.end()}
    {}

    db_dup_view(db_dup_view &&) = default;
    auto operator=(db_dup_view &&) -> db_dup_view & = default;

    auto begin() const
    {
        auto const &api = _cursor.get_deleter().api;
        return Iterator{api.get(), *_cursor, _key};
    }

    auto end() const
    {
        return std::default_sentinel;
    }

private:
    details::cursor_unique_ptr_t<LmdbApi> _cursor;
    std::vector<std::byte> _key;
};

}  // namespace lmdb
