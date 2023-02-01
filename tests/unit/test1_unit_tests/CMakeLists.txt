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

zephyr_compile_definitions(ZCBOR_STOP_ON_ERROR)

if (NOT VERBOSE AND CONFIG_MINIMAL_LIBC)
  # VERBOSE means including printk which doesn't build with these options.
  target_compile_options(zcbor PRIVATE -Wpedantic -Wconversion -Wall -Wextra)
endif()
