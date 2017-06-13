cmake_minimum_required(VERSION 3.6)
project(myimdb)

include_directories("include")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -Werror")

set(SOURCE_FILES main.c
        include/containers/vector.h
        include/storage/attribute.h
        include/storage/rowstore.h
        include/storage/schema.h
        include/containers/hash_table.h
        containers/hash_table.c
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

add_executable(myimdb ${SOURCE_FILES})