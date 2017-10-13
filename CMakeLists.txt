# this will make sure that one is using the clang compiler
set(CMAKE_CXX_COMPILER  "/usr/bin/clang++")
set(CMAKE_C_COMPILER "/usr/bin/clang")

project(gridstore)

set(CMAKE_BINARY_DIR          ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH    ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_TEMP_OUTPUT_DIR     ${CMAKE_SOURCE_DIR}/cmake)

message("exectuables are written to: ${EXECUTABLE_OUTPUT_PATH}")

# include third-party cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_TEMP_OUTPUT_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=c11 -Wall -Werror")

cmake_minimum_required(
    VERSION 3.5
)

include(
    "${CMAKE_SOURCE_DIR}/cmake/FindOS.cmake"
    "${CMAKE_SOURCE_DIR}/cmake/FindAPR.cmake"
)

find_package(APR     REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Curses  REQUIRED)
find_package(Doxygen REQUIRED)

include_directories(
    "/usr/local/include/"
    "include"
    "third-party"
    ${APR_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${CURSES_INCLUDE_DIR}
)

message("OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
message("OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
message("OPENSSL_ROOT_DIR: ${OPENSSL_ROOT_DIR}")

option(BUILD_DOCUMENTATION "API documentation requires Doxygen" ${DOXYGEN_FOUND})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR})

set(SOURCE_FILES
    include/async.h
    include/attr.h
    include/c11threads.h
    include/containers/list.h
    include/containers/vec.h
    include/frags/frag_host_vm.h
    include/error.h
    include/tuplet_field.h
    include/frag.h
    include/hash.h
    include/msg.h
    include/pred.h
    include/require.h
    include/schema.h
    include/gs.h
    include/tableimg.h
    include/tuplet.h
    include/operators/scan.h
    include/frag_printer.h
    include/frag_printers/console_printer.h
    include/unsafe.h
    include/timer.h
    include/field_type.h
    include/tuple.h
    include/inet/gs_server.h
    include/debug.h
    include/global.h
    include/inet/response.h
    include/grid.h
    include/interval.h
    include/tuple_field.h
    include/indexes/vindex.h
    include/indexes/vindexes/hash_vindex.h
    include/containers/freelist.h
    include/tuple_cursor.h
    include/indexes/hindex.h
    include/indexes/hindexes/lsearch_hindex.h
    include/grid_cursor.h
    include/containers/hashset.h
    include/routers/api/types/create/router.h
    include/utils.h
    include/gs_dispatcher.h
    include/gs_event.h
    include/gs_gridstore.h
    include/info.h
    include/gs_shell.h
    include/inet/gs_request.h
    include/routers/api/1.0/nodes.h
    include/routers/api/1.0/root.h
    include/gs_spinlock.h
    include/gs_http.h
    include/containers/gs_hash.h
    include/platform.h

    src/async.c
    src/attr.c
    src/containers/list.c
    src/containers/vec.c
    src/frags/frag_host_vm.c
    src/error.c
    src/tuplet_field.c
    src/frag.c
    src/hash.c
    src/pred.c
    src/schema.c
    src/tuplet.c
    src/operators/scan.c
    src/frag_printer.c
    src/frag_printers/console_printer.c
    src/unsafe.c
    src/timer.c
    src/field_type.c
    src/tuple.c
    src/inet/gs_server.c
    src/global.c
    src/inet/response.c
    src/grid.c
    src/interval.c
    src/tuple_field.c
    src/indexes/vindex.c
    src/indexes/vindexes/hash_vindex.c
    src/containers/freelist.c
    src/tuple_cursor.c
    src/indexes/hindex.c
    src/indexes/hindexes/lsearch_hindex.c
    src/grid_cursor.c
    src/containers/hashset.c
    src/utils.c
    src/gs_dispatcher.c
    src/gs_event.c
    src/gs_gridstore.c
    src/gs_shell.c
    src/inet/gs_request.c
    src/gs_spinlock.c
    src/routers/api/1.0/nodes.c
    src/routers/api/1.0/root.c
    src/gs_http.c
    src/containers/gs_hash.c

    third-party/json-parser/json.h
    third-party/json-parser/json.c
)

# add platform-specific code
IF (PROJECT_OS_LINUX)
    MESSAGE (STATUS "Operating system is Linux")
    set (SOURCE_FILES ${SOURCE_FILES} include/platforms/linux/platform.h)
ELSEIF (PROJECT_OS_OSX)
    MESSAGE (STATUS "Operating system is Apple MacOS X")
    set (SOURCE_FILES   ${SOURCE_FILES} include/platforms/macos/platform.h)
ENDIF (PROJECT_OS_LINUX)

add_executable(
    gridstore
    gridstore.c
    ${SOURCE_FILES}
)

IF (PROJECT_OS_LINUX)
    # add two aditional flags to call posix functions and to use the bsd library
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_XOPEN_SOURCE=700 -D_BSD_SOURCE")

    target_link_libraries(
            gridstore
            bsd
            m
            ${APR_LIBRARIES}
            ${APRUTIL_LIBRARIES}
            ${OPENSSL_LIBRARIES}
            pthread
    )
ELSEIF (PROJECT_OS_OSX)
   target_link_libraries(
            gridstore
            ${APR_LIBRARIES}
            ${APRUTIL_LIBRARIES}
            /usr/local/opt/argp-standalone/lib
            /usr/local/opt/argp-standalone/lib/libargp.a
            ${OPENSSL_LIBRARIES}
            pthread
    )
ENDIF (PROJECT_OS_LINUX)

if(DOXYGEN_FOUND)
    add_custom_target(
        doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)