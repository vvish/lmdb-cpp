add_executable(
    unit_tests
    test_create_environment.cpp 
    test_environment.cpp 
    test_transaction.cpp 
    test_key_value_traits.cpp
    test_error_handling_exceptions.cpp
    test_error_handling_expected.cpp
)

enable_testing()

set_target_properties(unit_tests PROPERTIES CXX_STANDARD 23)

target_link_libraries(unit_tests gtest gmock gtest_main lmdb)
target_compile_options(unit_tests PRIVATE -O0 --coverage -g)
target_link_options(unit_tests PRIVATE --coverage)

add_test(NAME unit_tests COMMAND unit_tests)
