#define CPP_LMDB_EXCEPTIONS_ENABLED
#include "cpp_lmdb/error.hpp"
#include "mocks.hpp"

// gtest
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// std
#include <type_traits>

namespace cpp_lmdb_tests
{
namespace
{
struct test_result {};

void test_function_noexcept() LMDB_NOEXCEPT{};
}  // namespace

static_assert(std::is_same_v<LMDB_RESULT(test_result), test_result>);
static_assert(!noexcept(test_function_noexcept()));

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
