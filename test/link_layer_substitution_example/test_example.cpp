#include "cpp_lmdb/error.hpp"

// test
#include "lmdb_mock.hpp"

// gtest
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// std
#include <expected>

namespace
{
// classes for technique demonstration
struct environment {
    MDB_env *env{};
};

auto make_environment() noexcept -> std::expected<environment, lmdb::error_t>
{
    MDB_env *env{};
    auto const result = mdb_env_create(&env);
    if (result != MDB_SUCCESS)
        return std::unexpected{lmdb::error_t{result}};

    return environment{env};
}

}  // namespace

namespace cpp_lmdb_tests
{
using namespace ::testing;  // NOLINT(google-build-using-namespace)

class test_lmdb_cpp : public Test {
    void SetUp() override
    {
        ::test::register_lmdb_mock(mock);
    }

    void TearDown() override
    {
        ::test::unregister_lmdb_mock();
    }

protected:
    StrictMock<::test::lmdb_mock> mock{};
};

TEST_F(test_lmdb_cpp, create_environment_error)
{
    EXPECT_CALL(mock, mdb_env_create(_)).WillOnce(Return(MDB_INVALID));

    auto const environment = make_environment();
    ASSERT_FALSE(environment);
    EXPECT_EQ(environment.error(), lmdb::error_t::invalid);
}

}  // namespace cpp_lmdb_tests
