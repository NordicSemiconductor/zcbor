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

set(py_command_pet
  cddl-gen
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/pet.cddl
  code
  --output-cmake ${PROJECT_BINARY_DIR}/pet.cmake
  -t Pet
  -d
  )
set(py_command_serial_recovery
  cddl-gen
  -c ${CMAKE_CURRENT_LIST_DIR}/../../cases/serial_recovery.cddl
  code
  --output-cmake ${PROJECT_BINARY_DIR}/serial_recovery.cmake
  -t Upload
  -d
  )
execute_process(
  COMMAND ${py_command_pet}
  COMMAND_ERROR_IS_FATAL ANY
  )
execute_process(
  COMMAND ${py_command_serial_recovery}
  COMMAND_ERROR_IS_FATAL ANY
  )

include(${PROJECT_BINARY_DIR}/pet.cmake)
include(${PROJECT_BINARY_DIR}/serial_recovery.cmake)

target_link_libraries(pet PRIVATE zephyr_interface)
target_link_libraries(serial_recovery PRIVATE zephyr_interface)
target_link_libraries(app PRIVATE pet serial_recovery)

zephyr_compile_options(-Werror)
