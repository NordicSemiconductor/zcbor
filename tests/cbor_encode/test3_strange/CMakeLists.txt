#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources})

set(py_command
  zcbor
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/strange.cddl
  --default-max-qty 6
  code
  --output-cmake ${PROJECT_BINARY_DIR}/strange.cmake
  --copy-sources
  -t
  NestedListMap NestedMapListMap
  Numbers
  Numbers2
  Strings
  Optional
  Union
  Map
  Level1
  Range
  ValueRange
  SingleBstr
  SingleInt
  SingleInt2
  Unabstracted
  QuantityRange
  DoubleMap
  -e
  )

execute_process(
  COMMAND ${py_command}
  COMMAND_ERROR_IS_FATAL ANY
  )

include(${PROJECT_BINARY_DIR}/strange.cmake)

target_link_libraries(strange PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE strange)

if (CANONICAL)
  target_compile_definitions(strange PUBLIC CDDL_CBOR_CANONICAL)
endif()

zephyr_compile_options(-Werror)
