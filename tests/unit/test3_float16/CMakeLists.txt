#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test3_unit_tests)

include(../../cmake/test_template.cmake)

FILE(GLOB zcbor_sources ../../../src/*.c)
target_sources(app PRIVATE
  ${zcbor_sources})

target_include_directories(app PRIVATE ../../../include)

if (VERBOSE)
  return() # Because the output volume is too large
endif()

if (CONFIG_NATIVE_APPLICATION AND NOT CONFIG_64BIT)
  set (LD_SIMULATION -m elf_i386)
endif()

set(LD ld)
if(NOT CONFIG_NATIVE_APPLICATION)
  set(LD ${CROSS_COMPILE_TARGET}-ld)
  if(DEFINED ENV{ZEPHYR_SDK_INSTALL_DIR})
    set(LD $ENV{ZEPHYR_SDK_INSTALL_DIR}/${CROSS_COMPILE_TARGET}/bin/${LD})
  endif()
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/include RESULT_VARIABLE err1)
execute_process(COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/floats.py ${PROJECT_BINARY_DIR} decode RESULT_VARIABLE err2)
execute_process(COMMAND ${LD} ${LD_SIMULATION} -r -b binary -o fp_bytes_decode.o fp_bytes_decode.bin WORKING_DIRECTORY ${PROJECT_BINARY_DIR} RESULT_VARIABLE err3)

target_link_libraries(app PRIVATE ${PROJECT_BINARY_DIR}/fp_bytes_decode.o)

execute_process(COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/floats.py ${PROJECT_BINARY_DIR} encode RESULT_VARIABLE err4)
execute_process(COMMAND ${LD} ${LD_SIMULATION} -r -b binary -o fp_bytes_encode.o fp_bytes_encode.bin WORKING_DIRECTORY ${PROJECT_BINARY_DIR} RESULT_VARIABLE err5)

target_link_libraries(app PRIVATE ${PROJECT_BINARY_DIR}/fp_bytes_encode.o)
