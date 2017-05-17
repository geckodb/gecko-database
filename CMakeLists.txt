cmake_minimum_required(VERSION 3.6)
project(myimdb)

include_directories("include")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(SOURCE_FILES main.c
        include/containers/vector.h
        include/storage/attribute.h
        include/storage/rowstore.h
        include/storage/schema.h
        include/defs.h
        include/error.h
        include/require.h
        containers/vector.c
        error.c
        require.c
        storage/attribute.c
        storage/schema.c)

add_executable(myimdb ${SOURCE_FILES})