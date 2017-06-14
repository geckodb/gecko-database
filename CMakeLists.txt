cmake_minimum_required(VERSION 3.6)
project(myimdb)

include_directories("include")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -Werror")

set(SOURCE_FILES main.c
        include/containers/vector.h
        include/storage/attribute.h
        include/storage/rowstore.h
        include/storage/schema.h
        include/containers/dictionaries/hash_table.h
        include/containers/dictionary.h
        containers/dictionary.c
        object.c
        include/object.h
        containers/dictionaries/hash_table.c
        containers/dictionaries/hash_tables/linear_hash_table.c
        include/containers/dictionaries/hash_tables/linear_hash_table.h
        include/macros.h
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

add_executable(myimdb ${SOURCE_FILES})