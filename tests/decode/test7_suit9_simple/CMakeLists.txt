#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test7_suit9_simple)
include(../../cmake/test_template.cmake)

set(py_command
  ${PYTHON_EXECUTABLE}
  ${CMAKE_CURRENT_LIST_DIR}/../../../zcbor/zcbor.py
  code
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/manifest9_simple.cddl
  --output-cmake ${PROJECT_BINARY_DIR}/manifest9_simple.cmake
  -t
  SUIT_Envelope
  SUIT_Manifest
  -d
  ${bit_arg}
  )

execute_process(
  COMMAND ${py_command}
  COMMAND_ERROR_IS_FATAL ANY
)

include(${PROJECT_BINARY_DIR}/manifest9_simple.cmake)

target_link_libraries(manifest9_simple PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE manifest9_simple)
