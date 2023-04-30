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

static_assert(std::is_same_v<
              LMDB_RESULT(test_result),
              std::expected<test_result, lmdb::error_t>>);
static_assert(noexcept(test_function_noexcept()));

TEST(error_handling_expected, report_error)
{
    auto const error = []() { LMDB_REPORT_ERROR(lmdb::error_t::bad_dbi); }();

    EXPECT_TRUE((
        std::
            is_same_v<decltype(error), const std::unexpected<lmdb::error_t>>));

    EXPECT_EQ(error.error(), lmdb::error_t::bad_dbi);
}

TEST(error_handling_expected, api_call_error)
{
    StrictMock<mocks::api> api;
    EXPECT_CALL(api, mdb_env_create(_)).WillOnce(Return(MDB_BAD_DBI));

    auto const result = [&api]() -> std::expected<void, lmdb::error_t> {
        MDB_env *env{};
        LMDB_CALL_API(api.mdb_env_create(&env));
        return {};
    }();

    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), lmdb::error_t::bad_dbi);
}

TEST(error_handling_expected, api_call_no_error)
{
    StrictMock<mocks::api> api;
    EXPECT_CALL(api, mdb_env_create(_)).WillOnce(Return(MDB_SUCCESS));

    auto const result = [&api]() -> std::expected<void, lmdb::error_t> {
        MDB_env *env{};
        LMDB_CALL_API(api.mdb_env_create(&env));
        return {};
    }();

    ASSERT_TRUE(result);
}
}  // namespace cpp_lmdb_tests
