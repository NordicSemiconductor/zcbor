#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test4_print_cbor)

set(REPO_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)

include(${REPO_ROOT}/tests/cmake/test_template.cmake)

FILE(GLOB zcbor_sources ${REPO_ROOT}/src/*.c)
target_sources(app PRIVATE
  ${zcbor_sources})

target_include_directories(app PRIVATE ${REPO_ROOT}/include)

zephyr_compile_definitions(ZCBOR_STOP_ON_ERROR)
zephyr_compile_definitions(ZCBOR_PRINT_CBOR)
