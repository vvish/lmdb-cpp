#pragma once

#include <span>

namespace lmdb
{

using byte_span = std::span<std::byte const>;

enum class read_only_t : bool { no = false, yes = true };
enum class allow_duplicates_t : bool { no = false, yes = true };

}  // namespace lmdb
