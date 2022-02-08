#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

set(PROJECT_NAME test1_suit)
include(../../cmake/test_template.cmake)

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
  ${bit_arg}
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
  ${bit_arg}
  )

execute_process(
  COMMAND ${py_command_e}
  COMMAND_ERROR_IS_FATAL ANY
)

execute_process(
  COMMAND ${py_command_d}
  COMMAND_ERROR_IS_FATAL ANY
)

include(${PROJECT_BINARY_DIR}/manifest3_encode.cmake)
include(${PROJECT_BINARY_DIR}/manifest3_decode.cmake)

target_link_libraries(manifest3_encode PRIVATE zephyr_interface)
target_link_libraries(manifest3_decode PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE manifest3_encode manifest3_decode)
