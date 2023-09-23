#include "lmdb_mock.hpp"

namespace test
{
namespace
{
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
lmdb_mock *global_mock{};
}  // namespace

void register_lmdb_mock(lmdb_mock &mock)
{
    global_mock = &mock;
}

void unregister_lmdb_mock()
{
    global_mock = nullptr;
}

extern "C" {
auto mdb_env_create(MDB_env **env) -> int
{
    return global_mock->mdb_env_create(env);
}

auto mdb_env_open(
    MDB_env *env, const char *path, unsigned int flags, mdb_mode_t mode) -> int
{
    return global_mock->mdb_env_open(env, path, flags, mode);
}
}
}  // namespace test
