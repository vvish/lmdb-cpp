add_executable(
    link_time_substitution
    test_example.cpp
    lmdb_mock.cpp
)

enable_testing()

set_target_properties(link_time_substitution PROPERTIES CXX_STANDARD 23)
target_link_libraries(link_time_substitution gtest gmock gtest_main lmdb)
add_test(NAME link_time_substitution COMMAND link_time_substitution)
