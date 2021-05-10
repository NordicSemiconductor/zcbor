#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

project(NONE)

add_subdirectory(../../ ${PROJECT_BINARY_DIR}/cddl_gen)

add_executable(fuzz_target main_entry.c)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/fuzz_input)

set(TEST_CASE "pet"  CACHE STRING  "Test case (pet or manifest12)")

if (${TEST_CASE} STREQUAL pet)
    target_cddl_source(fuzz_target ../cases/pet.cddl ENTRY_TYPES Pet DECODE)

    execute_process(
        COMMAND python3 ../../../scripts/cddl_gen.py
        --cddl ../../cases/pet.cddl
        convert
        --input ../../cases/pet0.yaml
        -t Pet
        --output ${PROJECT_BINARY_DIR}/fuzz_input/input0.cbor
        --output-as cbor
        )

    target_sources(fuzz_target PRIVATE fuzz_pet.c)

elseif (${TEST_CASE} STREQUAL manifest12)
    target_cddl_source(fuzz_target ../cases/manifest12.cddl DECODE
        ENTRY_TYPES SUIT_Envelope_Tagged SUIT_Command_Sequence SUIT_Envelope)

    foreach(n RANGE 0 5)
        execute_process(
        COMMAND python3 ../../../scripts/cddl_gen.py
        --cddl ../../cases/manifest12.cddl
        --default-max-qty 16
        convert
        --input ../../cases/manifest12_example${n}.cborhex
        -t SUIT_Envelope_Tagged
        --output ${PROJECT_BINARY_DIR}/fuzz_input/input${n}.cbor
        --output-as cbor
        )
    endforeach()

    target_sources(fuzz_target PRIVATE fuzz_manifest12.c)

else()
    message(FATAL_ERROR "Invalid test case")
endif()
