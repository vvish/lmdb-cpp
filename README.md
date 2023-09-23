# LMDB C++ bindings

## Description

Header-only wrapper for LMDB API

The code is a demo for *"Efficient and Reliable Wrapping of C APIs Using Modern C++"* presentation at *Cpp On Sea 2023* conference.

It offers minimalist functionality and primarily illustrates C++ features (up to C++23 standard) and design patterns applicable for wrapping C code.
Code is not "production ready" and is provided as is.

## Repository structure

Repository has the following primary directories:

* include/cpp_lmdb - library code
* test/unit - wrapper unit tests with compile-time LMDB mock injection
* test/link_time_substitution_example - example of unit tests with link-time LMDB mock injection
* test/integration - integration tests (bindings and LMDB)
