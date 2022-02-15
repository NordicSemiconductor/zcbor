#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

include(../../cmake/test_template.cmake)

set(py_command
  zcbor
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/senml.cddl
  code
  --output-cmake ${PROJECT_BINARY_DIR}/senml.cmake
  -t lwm2m_senml
  -e
  ${bit_arg}
  )

execute_process(
  COMMAND ${py_command}
  COMMAND_ERROR_IS_FATAL ANY
  )

include(${PROJECT_BINARY_DIR}/senml.cmake)

target_link_libraries(senml PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE senml)

