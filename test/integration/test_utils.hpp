#pragma once

// std
#include <ranges>
#include <vector>

namespace cpp_lmdb_tests
{

template <std::ranges::range ro_view>
auto get_all_values(ro_view const &view)
{
    return std::ranges::ref_view{view}
           | std::views::transform([](auto const &it) { return it.value(); })
           | std::ranges::to<std::vector>();
}

}  // namespace cpp_lmdb_tests
