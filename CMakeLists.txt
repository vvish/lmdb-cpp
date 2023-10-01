cmake_minimum_required(VERSION 3.9)

cmake_policy(SET CMP0048 NEW)

project(cpp_lmdb VERSION 0.1)

set(3P_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3p)

add_subdirectory(${3P_DIR}/googletest)
add_subdirectory(${3P_DIR}/lmdb_cmake)

if(CLANG-TIDY)
set(
    CMAKE_CXX_CLANG_TIDY 
    clang-tidy-15 
    -checks=*,-llvmlibc-*,-google-readability-avoid-underscore-in-googletest-name,-fuchsia-*,-altera-*
    -extra-arg=-std=c++20
    )
endif(CLANG-TIDY)

add_compile_options(-Wall -Wextra -fconcepts-diagnostics-depth=5)

include_directories(include)

add_subdirectory(test/unit)
add_subdirectory(test/integration)
add_subdirectory(test/link_time_substitution_example)

enable_testing()
