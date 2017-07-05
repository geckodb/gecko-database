cmake_minimum_required(VERSION 3.6)
project(myimdb)

include_directories("include")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=c11 -Wall -Werror")

set(CMAKE_BINARY_DIR
        ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH
        ${CMAKE_BINARY_DIR})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR})

set(SOURCE_FILES
        include/containers/vector.h
        include/storage/attribute.h
        include/storage/rowstore.h
        include/storage/schema.h
        include/containers/dictionaries/fixed_linear_hash_table.h
        include/containers/dict.h
        containers/dict.c
        containers/dictionaries/fixed_linear_hash_table.c
        include/macros.h
        hash.c
        include/pref.h
        conf.c
        include/conf.h
        pref.c
        include/hash.h
        include/msg.h
        include/defs.h
        include/error.h
        include/require.h
        containers/vector.c
        async.c
        include/c11threads.h
        include/storage/table.h
        include/async.h
        include/containers/list.h
        containers/list.c
        include/storage/memory.h
        storage/memory.c
        storage/table.c
        error.c
        require.c
        types.c
        include/types.h
        storage/attribute.c
        storage/schema.c)

add_executable(myimdb main.c ${SOURCE_FILES})
add_executable(sample_lin_hash samples/containers/dictionaries/linear_hash_table_sample.c ${SOURCE_FILES})
add_executable(sample_linuxwc samples/containers/dictionaries/linear_hash_table_linuxwc.c ${SOURCE_FILES})
add_executable(exp_os_paging experiments/vm/os_paging/launcher.c ${SOURCE_FILES})