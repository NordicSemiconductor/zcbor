#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

set_property(GLOBAL PROPERTY CSTD c11) # To avoid issues with c99 and -Wpedantic

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test4_senml)
include(../../cmake/test_template.cmake)

set(py_command
  zcbor
  code
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/senml.cddl
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

if (NOT VERBOSE)
  # VERBOSE means including printk which doesn't build with these options.
  target_compile_options(senml PRIVATE -Wpedantic -Wconversion)
endif()
