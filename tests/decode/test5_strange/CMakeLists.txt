#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

include(../../cmake/test_template.cmake)

set(py_command
  ${PYTHON_EXECUTABLE}
  ${CMAKE_CURRENT_LIST_DIR}/../../../zcbor/zcbor.py
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/strange.cddl
  --default-max-qty 6
  code
  --output-c ${PROJECT_BINARY_DIR}/src/strange.c
  --output-h ${PROJECT_BINARY_DIR}/include/strange.h
  --copy-sources
  -t NestedListMap NestedMapListMap
    Numbers
    Numbers2
    Strings
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
  --decode
  --git-sha-header
  ${bit_arg}
  )

execute_process(
  COMMAND
  ${py_command}
  COMMAND_ERROR_IS_FATAL ANY
)

target_sources(app PRIVATE
  ${PROJECT_BINARY_DIR}/src/strange.c
  ${PROJECT_BINARY_DIR}/src/zcbor_decode.c
  ${PROJECT_BINARY_DIR}/src/zcbor_common.c)

if (TEST_INDETERMINATE_LENGTH_ARRAYS)
  target_compile_definitions(app PUBLIC TEST_INDETERMINATE_LENGTH_ARRAYS)
endif()

target_include_directories(app PRIVATE
  ${PROJECT_BINARY_DIR}/include)
