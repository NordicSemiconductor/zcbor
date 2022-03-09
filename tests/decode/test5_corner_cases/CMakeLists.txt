#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

project(test5_strange)
include(../../cmake/test_template.cmake)

set(py_command
  ${PYTHON_EXECUTABLE}
  ${CMAKE_CURRENT_LIST_DIR}/../../../zcbor/zcbor.py
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/corner_cases.cddl
  --default-max-qty 6
  code
  --output-c ${PROJECT_BINARY_DIR}/src/corner_cases.c
  --output-h ${PROJECT_BINARY_DIR}/include/corner_cases.h
  --copy-sources
  -t NestedListMap NestedMapListMap
    Numbers
    Numbers2
    NumberMap
    Strings
    Prim2
    Optional
    Union
    Map
    EmptyMap
    Level1
    Range
    ValueRange
    SingleBstr
    SingleInt
    SingleInt2
    Unabstracted
    QuantityRange
    DoubleMap
    Floats
    Prelude
    CBORBstr
  --decode
  --git-sha-header
  ${bit_arg}
  )

execute_process(
  COMMAND
  ${py_command}
  COMMAND_ERROR_IS_FATAL ANY
)

add_library(strange
  ${PROJECT_BINARY_DIR}/src/corner_cases.c
  ${PROJECT_BINARY_DIR}/src/zcbor_decode.c
  ${PROJECT_BINARY_DIR}/src/zcbor_common.c)

target_include_directories(strange PUBLIC ${PROJECT_BINARY_DIR}/include)
target_link_libraries(strange PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE strange)

if (TEST_INDEFINITE_LENGTH_ARRAYS)
  target_compile_definitions(app PUBLIC TEST_INDEFINITE_LENGTH_ARRAYS)
endif()

if (NOT VERBOSE)
  # VERBOSE means including printk which doesn't build with these options.
  target_compile_options(strange PRIVATE -Wpedantic -Wconversion)
endif()
