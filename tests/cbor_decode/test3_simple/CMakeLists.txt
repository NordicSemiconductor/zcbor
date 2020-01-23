#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.8.2)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
add_subdirectory(../../.. ${PROJECT_BINARY_DIR}/cddl_gen)
project(NONE)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources})

target_cddl_source(app pet.cddl ENTRY_TYPES Pet)
target_cddl_source(app serial_recovery.cddl ENTRY_TYPES Upload)
