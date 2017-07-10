cmake_minimum_required(VERSION 3.5)
project(myimdb)

include_directories("include")
include_directories("/usr/local/include/")

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

message("OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
message("OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
message("OPENSSL_ROOT_DIR: ${OPENSSL_ROOT_DIR}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -std=c11 -Wall -Werror")

set(CMAKE_BINARY_DIR
        ${CMAKE_SOURCE_DIR}/bin)

set(EXECUTABLE_OUTPUT_PATH
        ${CMAKE_BINARY_DIR})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR})

set(SOURCE_FILES
        include/async.h
        include/attr.h
        include/c11threads.h
        include/conf.h
        include/containers/dict.h
        include/containers/dicts/hash_table.h
        include/containers/list.h
        include/containers/vector.h
        include/error.h
        include/field.h
        include/fragment.h
        include/hash.h
        include/msg.h
        include/pred.h
        include/pref.h
        include/require.h
        include/schema.h
        include/stdinc.h
        include/storage/attribute.h
        include/storage/memory.h
        include/storage/rowstore.h
        include/storage/schema.h
        include/storage/table.h
        include/tableimg.h
        include/tuplet.h

        async.c
        attr.c
        conf.c
        containers/dict.c
        containers/dicts/hash_table.c
        containers/list.c
        containers/vector.c
        error.c
        field.c
        fragment.c
        hash.c
        pred.c
        pref.c
        require.c
        schema.c
        storage/memory.c
        tableimg.c
        tuplet.c)

add_executable(
        myimdb main.c
        ${SOURCE_FILES})

add_executable(
        sample_lin_hash
        samples/containers/dictionaries/linear_hash_table_sample.c
        ${SOURCE_FILES})

add_executable(
        sample_linuxwc
        samples/containers/dictionaries/linear_hash_table_linuxwc.c
        ${SOURCE_FILES})

add_executable(
        exp_os_paging
        experiments/vm/os_paging/launcher.c
        ${SOURCE_FILES})

add_executable(
        tpch-convert utils/tpch_convert/launcher.c
        ${SOURCE_FILES})

add_executable(
        tableimg utils/tableimg/launcher.c
        ${SOURCE_FILES})

add_executable(
        dbstdio samples/core/fragment.c
        ${SOURCE_FILES})

add_executable(
        expr samples/core/pred.c
        ${SOURCE_FILES})

target_link_libraries(
        tpch-convert
        ${OPENSSL_LIBRARIES})

target_link_libraries(
        tableimg
        ${OPENSSL_LIBRARIES})

target_link_libraries(
        dbstdio
        ${OPENSSL_LIBRARIES})