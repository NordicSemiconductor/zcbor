#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

FILE(GLOB app_sources src/*.c)
FILE(GLOB zcbor_sources ../../../src/*.c)
target_sources(app PRIVATE
  ${app_sources}
  ${zcbor_sources})

target_include_directories(app PRIVATE ../../../include)

if (VERBOSE)
  target_compile_definitions(app PUBLIC ZCBOR_VERBOSE)
endif()

zephyr_compile_options(-Werror)
