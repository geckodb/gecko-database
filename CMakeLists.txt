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
    include/gs_async.h
    include/gs_attr.h
    include/gs_c11threads.h
    include/containers/gs_list.h
    include/containers/gs_vec.h
    include/frags/gs_frag_host_vm.h
    include/gs_error.h
    include/gs_tuplet_field.h
    include/gs_frag.h
    include/gs_hash.h
    include/gs_msg.h
    include/gs_pred.h
    include/gs_require.h
    include/gs_schema.h
    include/gs.h
    include/gs_tableimg.h
    include/gs_tuplet.h
    include/operators/gs_scan.h
    include/gs_frag_printer.h
    include/frag_printers/gs_console_printer.h
    include/gs_unsafe.h
    include/gs_timer.h
    include/gs_field_type.h
    include/gs_tuple.h
    include/inet/gs_server.h
    include/gs_debug.h
    include/gs_global.h
    include/inet/gs_response.h
    include/gs_grid.h
    include/gs_interval.h
    include/gs_tuple_field.h
    include/indexes/gs_vindex.h
    include/indexes/vindexes/gs_hash_vindex.h
    include/containers/gs_freelist.h
    include/gs_tuple_cursor.h
    include/indexes/gs_hindex.h
    include/indexes/hindexes/gs_lsearch_hindex.h
    include/gs_grid_cursor.h
    include/containers/gs_hashset.h
    include/routers/api/types/create/gs_router.h
    include/gs_utils.h
    include/gs_dispatcher.h
    include/gs_event.h
    include/gs_gridstore.h
    include/gs_info.h
    include/gs_shell.h
    include/inet/gs_request.h
    include/routers/api/1.0/gs_nodes.h
    include/gs_spinlock.h
    include/gs_http.h
    include/containers/gs_hash.h
    include/gs_platform.h

    src/gs_async.c
    src/gs_attr.c
    src/containers/gs_list.c
    src/containers/gs_vec.c
    src/frags/gs_frag_host_vm.c
    src/gs_error.c
    src/gs_tuplet_field.c
    src/gs_frag.c
    src/gs_hash.c
    src/gs_pred.c
    src/gs_schema.c
    src/gs_tuplet.c
    src/operators/gs_scan.c
    src/gs_frag_printer.c
    src/frag_printers/gs_console_printer.c
    src/gs_unsafe.c
    src/gs_timer.c
    src/gs_field_type.c
    src/gs_tuple.c
    src/inet/gs_server.c
    src/gs_global.c
    src/inet/gs_response.c
    src/gs_grid.c
    src/gs_interval.c
    src/gs_tuple_field.c
    src/indexes/gs_vindex.c
    src/indexes/vindexes/gs_hash_vindex.c
    src/containers/gs_freelist.c
    src/gs_tuple_cursor.c
    src/indexes/gs_hindex.c
    src/indexes/hindexes/gs_lsearch_hindex.c
    src/gs_grid_cursor.c
    src/containers/gs_hashset.c
    src/gs_utils.c
    src/gs_dispatcher.c
    src/gs_event.c
    src/gs_gridstore.c
    src/gs_shell.c
    src/inet/gs_request.c
    src/gs_spinlock.c
    src/routers/api/1.0/gs_nodes.c
    src/gs_http.c
    src/containers/gs_hash.c

    third-party/json-parser/json.h
    third-party/json-parser/json.c
)

# add platform-specific code
IF (PROJECT_OS_LINUX)
    MESSAGE (STATUS "Operating system is Linux")
    set (SOURCE_FILES ${SOURCE_FILES} include/platforms/linux/gs_platform.h)
ELSEIF (PROJECT_OS_OSX)
    MESSAGE (STATUS "Operating system is Apple MacOS X")
    set (SOURCE_FILES   ${SOURCE_FILES} include/platforms/macos/gs_platform.h)
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