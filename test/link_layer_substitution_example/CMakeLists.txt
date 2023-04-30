add_executable(
    link_layer_substitution
    test_example.cpp
    lmdb_mock.cpp
)

enable_testing()

set_target_properties(link_layer_substitution PROPERTIES CXX_STANDARD 23)
target_link_libraries(link_layer_substitution gtest gmock gtest_main lmdb)
add_test(NAME link_layer_substitution COMMAND link_layer_substitution)
