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
        include/containers/vector.h
        include/frags/frag_host_vm.h
        include/error.h
        include/field.h
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
        include/grid_table.h
        include/tuple.h
        include/inet/server.h
        include/debug.h
        include/global.h
        include/inet/request.h
        include/inet/response.h
        include/mvm.h

        async.c
        attr.c
        conf.c
        containers/dict.c
        containers/dicts/hash_table.c
        containers/list.c
        containers/vector.c
        frags/frag_host_vm.c
        error.c
        field.c
        frag.c
        hash.c
        pred.c
        pref.c
        require.c
        schema.c
        storage/memory.c
        tuplet.c
        operators/scan.c
        mondrian.c
        frag_printer.c
        frag_printers/console_printer.c
        unsafe.c
        timer.c
        field_type.c
        grid_table.c
        tuple.c
        inet/server.c
        debug.c
        global.c
        inet/request.c
        inet/response.c
        mvm.c
        )

add_executable(
        gs main.c
        shell.c
        include/shell.h
        ${SOURCE_FILES})

add_executable(
        mondrian
        playground/mondrian_virtual_machine.c
        ${SOURCE_FILES})

target_link_libraries(gs ${CURSES_LIBRARIES})

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

add_executable(
        tpch-convert utils/tpch_convert/launcher.c
        utils/tpch_convert/common.h
        utils/tpch_convert/types.h
        utils/tpch_convert/tables/customer.h
        utils/tpch_convert/tables/lineitem.h
        utils/tpch_convert/tables/nation.h
        utils/tpch_convert/tables/orders.h
        utils/tpch_convert/tables/part.h
        utils/tpch_convert/tables/partsupp.h
        utils/tpch_convert/tables/region.h
        utils/tpch_convert/tables/supplier.h
        ${SOURCE_FILES})

#add_executable(
#        tableimg utils/tableimg/launcher.c
#        ${SOURCE_FILES})

add_executable(
        dbstdio samples/stddb.c
        ${SOURCE_FILES})

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
        gs
        ${OPENSSL_LIBRARIES})

target_link_libraries(
        mondrian
        ${OPENSSL_LIBRARIES})

target_link_libraries(
        tpch-convert
        ${OPENSSL_LIBRARIES})

#target_link_libraries(
#        tableimg
#        ${OPENSSL_LIBRARIES})

target_link_libraries(
        dbstdio
        ${OPENSSL_LIBRARIES})

if(DOXYGEN_FOUND)
    #configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
            )
endif(DOXYGEN_FOUND)