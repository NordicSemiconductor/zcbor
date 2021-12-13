#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

project(NONE)

add_executable(fuzz_target main_entry.c)

target_compile_options(fuzz_target PUBLIC -Werror)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/fuzz_input)

set(TEST_CASE "pet"  CACHE STRING  "Test case (pet or manifest12)")

if (${TEST_CASE} STREQUAL pet)
    execute_process(
        COMMAND zcbor
        -c ${CMAKE_CURRENT_LIST_DIR}/../cases/pet.cddl
        code
        --output-cmake ${PROJECT_BINARY_DIR}/pet.cmake
        -t Pet
        -d
        )

    execute_process(
        COMMAND python3 ../../../zcbor/zcbor.py
        --cddl ../../cases/pet.cddl
        convert
        --input ../../cases/pet0.yaml
        -t Pet
        --output ${PROJECT_BINARY_DIR}/fuzz_input/input0.cbor
        --output-as cbor
        )

    target_sources(fuzz_target PRIVATE fuzz_pet.c)
    include(${PROJECT_BINARY_DIR}/pet.cmake)
    target_link_libraries(fuzz_target PRIVATE pet)

elseif (${TEST_CASE} STREQUAL manifest12)
    execute_process(
        COMMAND zcbor
        -c ${CMAKE_CURRENT_LIST_DIR}/../cases/manifest12.cddl
        code
        --output-cmake ${PROJECT_BINARY_DIR}/manifest12.cmake
        -t SUIT_Envelope_Tagged SUIT_Command_Sequence SUIT_Envelope
        -d
        )

    foreach(n RANGE 0 5)
        execute_process(
        COMMAND python3 ../../../zcbor/zcbor.py
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
    include(${PROJECT_BINARY_DIR}/manifest12.cmake)
    target_link_libraries(fuzz_target PRIVATE manifest12)

else()
    message(FATAL_ERROR "Invalid test case")
endif()
