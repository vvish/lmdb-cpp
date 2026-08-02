#include <filesystem>
#define CPP_LMDB_EXCEPTIONS_ENABLED
#include "cpp_lmdb/environment.hpp"
#include "cpp_lmdb/error.hpp"
#include "mocks.hpp"

// gtest
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// std
#include <type_traits>
#include <utility>

namespace cpp_lmdb_tests
{
namespace
{
struct test_result {};

[[maybe_unused]] auto test_function_noexcept() LMDB_NOEXCEPT {};
}  // namespace

static_assert(std::is_same_v<LMDB_RESULT(test_result), test_result>);
static_assert(!noexcept(test_function_noexcept()));

static_assert(std::is_same_v<
              decltype(lmdb::make_environment<lmdb::env_flags_t::none, 1>(
                  "", lmdb::default_file_mode)),
              lmdb::rw_environment<>>);

using db_trait
    = lmdb::unique_key<lmdb::trivial_trait<int>, lmdb::trivial_trait<int>>;

static_assert(std::is_same_v<
              decltype(std::declval<lmdb::ro_environment<>>()
                           .template open_ro_db<db_trait>("db")),
              lmdb::ro_db<db_trait, lmdb::details::api>>);

TEST(error_handling_exceptions, report_error)
{
    try {
        LMDB_REPORT_ERROR(lmdb::error_t::bad_dbi);
        FAIL();
    } catch (lmdb::lmdb_exception const &ex) {
        EXPECT_EQ(ex.error(), lmdb::error_t::bad_dbi);
    }
}

TEST(error_handling_exceptions, api_call_exception)
{
    MDB_env *env{};

    StrictMock<mocks::api> api;
    EXPECT_CALL(api, mdb_env_create(_)).WillOnce(Return(MDB_BAD_DBI));

    try {
        LMDB_CALL_API(api.mdb_env_create(&env));
        FAIL();
    } catch (lmdb::lmdb_exception const &ex) {
        EXPECT_EQ(ex.error(), lmdb::error_t::bad_dbi);
    }
}

TEST(error_handling_exceptions, api_call_no_exception)
{
    MDB_env *env{};

    StrictMock<mocks::api> api;
    EXPECT_CALL(api, mdb_env_create(_)).WillOnce(Return(MDB_SUCCESS));

    EXPECT_NO_THROW(LMDB_CALL_API(api.mdb_env_create(&env)));
}

}  // namespace cpp_lmdb_tests
