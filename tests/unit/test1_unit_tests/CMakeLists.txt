#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

set_property(GLOBAL PROPERTY CSTD c11) # To avoid issues with c99 and -Wpedantic

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test1_unit_tests)
include(../../cmake/test_template.cmake)

FILE(GLOB zcbor_sources ../../../src/*.c)
zephyr_library_named(zcbor)
zephyr_library_sources(${zcbor_sources})

target_include_directories(app PRIVATE ../../../include)
target_include_directories(zcbor PRIVATE ../../../include)

target_link_libraries(app PRIVATE zcbor)

target_compile_options(zcbor PRIVATE -Wpedantic -Wconversion -Wall -Wextra -Wdouble-promotion)

file(READ ${CMAKE_CURRENT_LIST_DIR}/../../../zcbor/VERSION ZCBOR_VERSION_STR)
string(REPLACE "." ";" ZCBOR_VERSION_SPLIT ${ZCBOR_VERSION_STR})
list(GET ZCBOR_VERSION_SPLIT 0 ZCBOR_VERSION_MAJOR)
list(GET ZCBOR_VERSION_SPLIT 1 ZCBOR_VERSION_MINOR)
list(GET ZCBOR_VERSION_SPLIT 2 ZCBOR_VERSION_BUGFIX)
math(EXPR ZCBOR_VERSION "(${ZCBOR_VERSION_MAJOR} << 24) + (${ZCBOR_VERSION_MINOR} << 16) + (${ZCBOR_VERSION_BUGFIX} << 8)")

zephyr_compile_definitions(
  ZCBOR_STOP_ON_ERROR
  TEST_ZCBOR_VERSION_STR="${ZCBOR_VERSION_STR}"
  TEST_ZCBOR_VERSION=${ZCBOR_VERSION}
  TEST_ZCBOR_VERSION_MAJOR=${ZCBOR_VERSION_MAJOR}
  TEST_ZCBOR_VERSION_MINOR=${ZCBOR_VERSION_MINOR}
  TEST_ZCBOR_VERSION_BUGFIX=${ZCBOR_VERSION_BUGFIX})
