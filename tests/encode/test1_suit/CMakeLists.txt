#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test1_suit)
include(../../cmake/test_template.cmake)

set(py_command
  zcbor
  code
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/manifest3.cddl
  --output-cmake ${PROJECT_BINARY_DIR}/manifest3.cmake
  -t
  SUIT_Outer_Wrapper
  SUIT_Command_Sequence
  SUIT_Authentication_Wrapper
  -e -d
  ${bit_arg}
  )

execute_process(
  COMMAND ${py_command}
  COMMAND_ERROR_IS_FATAL ANY
)

include(${PROJECT_BINARY_DIR}/manifest3.cmake)

target_link_libraries(manifest3 PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE manifest3)
