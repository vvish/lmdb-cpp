set(LMDB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lmdb/libraries/liblmdb)

add_library(lmdb
    ${CMAKE_CURRENT_SOURCE_DIR}/../lmdb/libraries/liblmdb/lmdb.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../lmdb/libraries/liblmdb/mdb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../lmdb/libraries/liblmdb/midl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../lmdb/libraries/liblmdb/midl.c)

target_include_directories(lmdb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../lmdb/libraries/liblmdb)
