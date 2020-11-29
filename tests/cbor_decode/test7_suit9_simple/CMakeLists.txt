#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.8.2)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
add_subdirectory(../../.. ${PROJECT_BINARY_DIR}/cddl_gen)
project(NONE)

target_cddl_source(app manifest9_simple.cddl
  ENTRY_TYPES
  SUIT_Envelope
  SUIT_Manifest
  DECODE
  ${VERBOSE}
  ${CANONICAL}
  )

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources}
  )

zephyr_compile_options(-Werror)
