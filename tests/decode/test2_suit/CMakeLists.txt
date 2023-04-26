#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

set_property(GLOBAL PROPERTY CSTD c11) # To avoid issues with c99 and -Wpedantic

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test2_suit)
include(../../cmake/test_template.cmake)

set(py_command
  zcbor
  code
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/manifest2.cddl
  --output-cmake ${PROJECT_BINARY_DIR}/manifest2.cmake
  -t
  SUIT_Outer_Wrapper
  SUIT_Command_Sequence
  SUIT_Authentication_Wrapper
  -d
  ${bit_arg}
  )

execute_process(
  COMMAND
  ${py_command}
  COMMAND_ERROR_IS_FATAL ANY
  )

include(${PROJECT_BINARY_DIR}/manifest2.cmake)

target_link_libraries(manifest2 PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE manifest2)

if (NOT VERBOSE)
  # VERBOSE means including printk which doesn't build with these options.
  target_compile_options(manifest2 PRIVATE -Wpedantic -Wconversion -Wdouble-promotion)
endif()
