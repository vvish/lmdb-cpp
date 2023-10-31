#pragma once

// std
#include <algorithm>
#include <ranges>
#include <vector>

namespace cpp_lmdb_tests
{

template <std::ranges::range R>
auto to_vector(R &&range)
{
    std::vector<std::ranges::range_value_t<R>> result;
    std::ranges::copy(std::forward<R>(range), std::back_inserter(result));

    return result;
}

template<typename ro_view>
auto get_all_values(ro_view const &view)
{
    auto const values
        = std::ranges::ref_view{view}
          | std::views::transform([](auto const &it) { return it.value(); });

    return to_vector(values);
}

}  // namespace cpp_lmdb_tests
