cmake_minimum_required(VERSION 3.9)

cmake_policy(SET CMP0048 NEW)

project(cpp_lmdb VERSION 0.1)

set (CMAKE_CXX_STANDARD 23)

set(3P_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3p)

add_subdirectory(${3P_DIR}/lmdb_cmake)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -fconcepts-diagnostics-depth=5)
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    add_compile_options(-Wall -Wextra)
    add_compile_options(-std=c++2b -stdlib=libc++)
    add_link_options(-stdlib=libc++)
endif()

add_subdirectory(${3P_DIR}/googletest)

if(CLANG-TIDY)
set(
    CMAKE_CXX_CLANG_TIDY
    clang-tidy-17
    -checks=*,-llvmlibc-*,-google-readability-avoid-underscore-in-googletest-name,-fuchsia-*,-altera-*
    -extra-arg=-std=c++23
    )
endif(CLANG-TIDY)

include_directories(include)

add_subdirectory(test/unit)
add_subdirectory(test/integration)
add_subdirectory(test/link_time_substitution_example)

enable_testing()
