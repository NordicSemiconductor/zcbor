#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

set(py_command
  ${PYTHON_EXECUTABLE}
  ${CMAKE_CURRENT_LIST_DIR}/../../../cddl_gen/cddl_gen.py
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/manifest14.cddl
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/cose.cddl
  --default-max-qty 4
  code
  --output-cmake ${PROJECT_BINARY_DIR}/manifest14.cmake
  -t
  SUIT_Envelope_Tagged
  SUIT_Envelope
  SUIT_Manifest
  SUIT_Command_Sequence
  SUIT_Common_Sequence
  -d
  )

execute_process(
  COMMAND
  ${py_command}
  COMMAND_ERROR_IS_FATAL ANY
)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources}
  )

include(${PROJECT_BINARY_DIR}/manifest14.cmake)

target_link_libraries(manifest14 PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE manifest14)

zephyr_compile_options(-Werror)
