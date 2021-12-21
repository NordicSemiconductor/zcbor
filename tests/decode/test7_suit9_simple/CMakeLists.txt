#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

set(py_command
  ${PYTHON_EXECUTABLE}
  ${CMAKE_CURRENT_LIST_DIR}/../../../zcbor/zcbor.py
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/manifest9_simple.cddl
  code
  --output-cmake ${PROJECT_BINARY_DIR}/manifest9_simple.cmake
  -t
  SUIT_Envelope
  SUIT_Manifest
  -d
  )

execute_process(
  COMMAND ${py_command}
  COMMAND_ERROR_IS_FATAL ANY
)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources}
  )

include(${PROJECT_BINARY_DIR}/manifest9_simple.cmake)

target_link_libraries(manifest9_simple PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE manifest9_simple)

zephyr_compile_options(-Werror)
