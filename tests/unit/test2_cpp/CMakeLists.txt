#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test2_unit_tests)
include(../../cmake/test_template.cmake)

FILE(GLOB zcbor_sources ../../../src/*.c)
target_sources(app PRIVATE
  ${zcbor_sources})

target_include_directories(app PRIVATE ../../../include)

zephyr_compile_definitions(ZCBOR_STOP_ON_ERROR)
