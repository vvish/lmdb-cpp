add_executable(
    integrations_tests
    test_db_int_keys_and_values.cpp
    test_db_string_keys_and_values.cpp
)

enable_testing()

set_target_properties(integrations_tests PROPERTIES CXX_STANDARD 23)
target_link_libraries(integrations_tests gtest gmock gtest_main lmdb)
add_test(NAME integrations_tests COMMAND integrations_tests)
