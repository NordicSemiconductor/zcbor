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
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/pet.cddl
  code
  --output-cmake ${PROJECT_BINARY_DIR}/pet.cmake
  -t Pet
  -e
  )

execute_process(
  COMMAND ${py_command}
  COMMAND_ERROR_IS_FATAL ANY
  )

include(${PROJECT_BINARY_DIR}/pet.cmake)

target_link_libraries(pet PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE pet)

if (CANONICAL)
  target_compile_definitions(pet PUBLIC ZCBOR_CANONICAL)
endif()

zephyr_compile_options(-Werror)
