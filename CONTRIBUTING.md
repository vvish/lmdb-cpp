# Contibution to the project

## General

Contributions in the form of PRs, open issues or general feedback are welcomed.

## C++ standard

Code was intentionally developed to be primarily based on `C++20` standard but also demonstrates some `C++23` features:
- std::expected
- std::to_underlying

There is an [issue](https://github.com/vvish/lmdb-cpp/issues/10) opened to investigate downgrade path to `C++20`
standard, but, for now, `C++23` is required

The set of used language features and standard library facilities are supported by `GCC-12` and newer.

## Build

Library is header-only but uses `cmake` for tests and examples.
To build the project in `Release` version in some build directory the following commands can be used:

```sh
mkdir <some/build/dir> && cd <some/build/dir>
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-12 -DCMAKE_C_COMPILER=gcc-12 <path/to/cpp-lmdb>
cmake --build . -- -j
```

## Test

Tests can be invoked from the build directory using `ctest` utility:

```sh
ctest
```

## PR verification

Verification of PRs is performed by GitHub Actions that build and run tests.
Code coverage is uploaded to Codecov and reported as a comment in a PR conversation.
As a rule, coverage should not be reduced by submitted changes.


