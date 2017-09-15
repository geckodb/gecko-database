cmake_minimum_required(VERSION 3.5)
project(gridstore)

include_directories("include")
include_directories("/usr/local/include/")

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

message("OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
message("OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
message("OPENSSL_ROOT_DIR: ${OPENSSL_ROOT_DIR}")

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

find_package(Doxygen)
option(BUILD_DOCUMENTATION "API documentation requires Doxygen" ${DOXYGEN_FOUND})

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
        include/containers/vec.h
        include/frags/frag_host_vm.h
        include/error.h
        include/tuplet_field.h
        include/frag.h
        include/hash.h
        include/msg.h
        include/pred.h
        include/pref.h
        include/require.h
        include/schema.h
        include/stdinc.h
        include/storage/memory.h
        include/tableimg.h
        include/tuplet.h
        include/operators/scan.h
        include/mondrian.h
        include/frag_printer.h
        include/frag_printers/console_printer.h
        include/unsafe.h
        include/timer.h
        include/field_type.h
        include/tuple.h
        include/inet/server.h
        include/debug.h
        include/global.h
        include/inet/request.h
        include/inet/response.h
        include/mvm.h
        include/progpool.h

        src/async.c
        src/attr.c
        src/conf.c
        src/containers/dict.c
        src/containers/dicts/hash_table.c
        src/containers/list.c
        src/containers/vec.c
        src/frags/frag_host_vm.c
        src/error.c
        src/tuplet_field.c
        src/frag.c
        src/hash.c
        src/pred.c
        src/pref.c
        src/schema.c
        src/storage/memory.c
        src/tuplet.c
        src/operators/scan.c
        src/mondrian.c
        src/frag_printer.c
        src/frag_printers/console_printer.c
        src/unsafe.c
        src/timer.c
        src/field_type.c
        src/tuple.c
        src/inet/server.c
        src/global.c
        src/inet/request.c
        src/inet/response.c
        src/mvm.c
        src/progpool.c
        src/grid.c
        include/grid.h
        src/interval.c
        include/interval.h
        src/tuple_field.c
        include/tuple_field.h
        src/indexes/vindex.c
        include/indexes/vindex.h
        include/indexes/vindexes/hash_vindex.h
        src/indexes/vindexes/hash_vindex.c
        include/containers/freelist.h
        src/containers/freelist.c
        include/tuple_cursor.h
        src/tuple_cursor.c
        include/indexes/hindex.h
        src/indexes/hindex.c
        include/indexes/hindexes/lsearch_hindex.h
        src/indexes/hindexes/lsearch_hindex.c
        src/grid_cursor.c
        include/grid_cursor.h
        src/containers/hashset.c
        include/containers/hashset.h
        include/routers/api/types/create/router.h
        )

add_executable(
        mondrian
        playground/mondrian_virtual_machine.c
        ${SOURCE_FILES})

add_executable(
        gridtables
        playground/mondrian_grid_tables.c
        ${SOURCE_FILES})

add_executable(
        gs-ycsb
        playground/grid_store_ycsb.c
        ${SOURCE_FILES})

add_executable(
        sample_lin_hash
        samples/containers/dictionaries/linear_hash_table_sample.c
        ${SOURCE_FILES})

target_link_libraries(
        sample_lin_hash
        ${OPENSSL_LIBRARIES})

add_executable(
        sample_linuxwc
        samples/containers/dictionaries/linear_hash_table_linuxwc.c
        ${SOURCE_FILES})

target_link_libraries(
        sample_linuxwc
        ${OPENSSL_LIBRARIES})

#add_executable(
#        exp_os_paging
#        experiments/vm/os_paging/launcher.c
#        ${SOURCE_FILES})

#target_link_libraries(
#        exp_os_paging
#        ${OPENSSL_LIBRARIES})

#add_executable(
#        tpch-convert utils/tpch_convert/launcher.c
#        utils/tpch_convert/common.h
#        utils/tpch_convert/types.h
#        utils/tpch_convert/tables/customer.h
#        utils/tpch_convert/tables/lineitem.h
#        utils/tpch_convert/tables/nation.h
#        utils/tpch_convert/tables/orders.h
#        utils/tpch_convert/tables/part.h
#        utils/tpch_convert/tables/partsupp.h
#        utils/tpch_convert/tables/region.h
#        utils/tpch_convert/tables/supplier.h
#        ${SOURCE_FILES})

#add_executable(
#        tableimg utils/tableimg/launcher.c
#        ${SOURCE_FILES})

add_executable(
        buffmgr samples/buffmgr/launcher.c
        ${SOURCE_FILES})

target_link_libraries(
        buffmgr
        ${OPENSSL_LIBRARIES})

add_executable(
        expr samples/core/pred.c
        ${SOURCE_FILES})

target_link_libraries(
        expr
        ${OPENSSL_LIBRARIES})

target_link_libraries(
        mondrian
        ${OPENSSL_LIBRARIES})

target_link_libraries(
        gridtables
        ${OPENSSL_LIBRARIES})

target_link_libraries(
        gs-ycsb
        ${OPENSSL_LIBRARIES})

#target_link_libraries(
#        tpch-convert
#        ${OPENSSL_LIBRARIES})

#target_link_libraries(
#        tableimg
#        ${OPENSSL_LIBRARIES})

if(DOXYGEN_FOUND)
    #configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
            )
endif(DOXYGEN_FOUND)