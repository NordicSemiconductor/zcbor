#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

set_property(GLOBAL PROPERTY CSTD c11) # To avoid issues with c99 and -Wpedantic

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test5_corner_cases)
include(../../cmake/test_template.cmake)

set(py_command
  ${PYTHON_EXECUTABLE}
  ${CMAKE_CURRENT_LIST_DIR}/../../../zcbor/zcbor.py
  code
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/corner_cases.cddl
  --default-max-qty 6
  --output-c ${PROJECT_BINARY_DIR}/src/corner_cases.c
  --output-h ${PROJECT_BINARY_DIR}/include/corner_cases.h
  --copy-sources
  -t NestedListMap NestedMapListMap
    Numbers
    Numbers2
    TaggedUnion
    NumberMap
    Strings
    Simple2
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
    Floats2
    Floats3
    Prelude
    CBORBstr
    MapLength
    UnionInt1
    UnionInt2
    Intmax1
    Intmax2
    Intmax3
    InvalidIdentifiers
    Uint64List
    BstrSize
  --decode
  --git-sha-header
  --short-names
  ${bit_arg}
  )

execute_process(
  COMMAND
  ${py_command}
  COMMAND_ERROR_IS_FATAL ANY
)

add_library(corner_cases
  ${PROJECT_BINARY_DIR}/src/corner_cases.c
  ${PROJECT_BINARY_DIR}/src/zcbor_decode.c
  ${PROJECT_BINARY_DIR}/src/zcbor_common.c)

target_include_directories(corner_cases PUBLIC ${PROJECT_BINARY_DIR}/include)
target_link_libraries(corner_cases PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE corner_cases)

if (TEST_INDEFINITE_LENGTH_ARRAYS)
  target_compile_definitions(app PUBLIC TEST_INDEFINITE_LENGTH_ARRAYS)
endif()

if (NOT VERBOSE AND CONFIG_MINIMAL_LIBC)
  # VERBOSE means including printk which doesn't build with these options.
  target_compile_options(corner_cases PRIVATE -Wpedantic -Wconversion -Wdouble-promotion)
endif()
