#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

set(py_command_e
  zcbor
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/manifest3.cddl
  code
  --output-cmake ${PROJECT_BINARY_DIR}/manifest3_encode.cmake
  -t
  SUIT_Outer_Wrapper
  SUIT_Command_Sequence
  SUIT_Authentication_Wrapper
  -e
  --oc ${PROJECT_BINARY_DIR}/src/manifest3_encode.c
  --oh ${PROJECT_BINARY_DIR}/include/manifest3_encode.h
  --output-h-types ${PROJECT_BINARY_DIR}/include/manifest3_types.h

  )

set(py_command_d
  zcbor
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/manifest3.cddl
  code
  --output-cmake ${PROJECT_BINARY_DIR}/manifest3_decode.cmake
  -t
  SUIT_Outer_Wrapper
  SUIT_Command_Sequence
  SUIT_Authentication_Wrapper
  -d
  --oc ${PROJECT_BINARY_DIR}/src/manifest3_decode.c
  --oh ${PROJECT_BINARY_DIR}/include/manifest3_decode.h
  --output-h-types ${PROJECT_BINARY_DIR}/include/manifest3_types.h
  )

execute_process(
  COMMAND ${py_command_e}
  COMMAND_ERROR_IS_FATAL ANY
)

execute_process(
  COMMAND ${py_command_d}
  COMMAND_ERROR_IS_FATAL ANY
)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources}
  )

include(${PROJECT_BINARY_DIR}/manifest3_encode.cmake)
include(${PROJECT_BINARY_DIR}/manifest3_decode.cmake)

target_link_libraries(manifest3_encode PRIVATE zephyr_interface)
target_link_libraries(manifest3_decode PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE manifest3_encode manifest3_decode)

if (CANONICAL)
  target_compile_definitions(manifest3_encode PUBLIC CDDL_CBOR_CANONICAL)
endif()

zephyr_compile_options(-Werror)
