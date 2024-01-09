#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

project(zcbor_fuzz)

add_executable(fuzz_target main_entry.c)

target_compile_options(fuzz_target PUBLIC -Werror)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/fuzz_input)

set(TEST_CASE "pet"  CACHE STRING  "Test case (pet or manifest12)")

if (${TEST_CASE} STREQUAL pet)
    execute_process(
        COMMAND zcbor
        code
        -c ${CMAKE_CURRENT_LIST_DIR}/../cases/pet.cddl
        --output-cmake ${PROJECT_BINARY_DIR}/pet.cmake
        -t Pet
        -d
        )

    execute_process(
        COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/../../zcbor/zcbor.py
        convert
        --cddl ${CMAKE_CURRENT_LIST_DIR}/../cases/pet.cddl
        --input ${CMAKE_CURRENT_LIST_DIR}/../cases/pet0.yaml
        -t Pet
        --yaml-compatibility
        --output ${PROJECT_BINARY_DIR}/fuzz_input/input0.cbor
        --output-as cbor
        )

    target_sources(fuzz_target PRIVATE ${CMAKE_CURRENT_LIST_DIR}/fuzz_pet.c)
    include(${PROJECT_BINARY_DIR}/pet.cmake)
    target_link_libraries(fuzz_target PRIVATE pet)

elseif (${TEST_CASE} STREQUAL manifest12)
    execute_process(
        COMMAND zcbor
        code
        -c ${CMAKE_CURRENT_LIST_DIR}/../cases/manifest12.cddl
        --output-cmake ${PROJECT_BINARY_DIR}/manifest12.cmake
        -t SUIT_Envelope_Tagged SUIT_Command_Sequence SUIT_Envelope
        -d
        )

    foreach(n RANGE 0 5)
        execute_process(
        COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/../../zcbor/zcbor.py
        convert
        --cddl ${CMAKE_CURRENT_LIST_DIR}/../cases/manifest12.cddl
        --default-max-qty 16
        --input ${CMAKE_CURRENT_LIST_DIR}/../cases/manifest12_example${n}.cborhex
        -t SUIT_Envelope_Tagged
        --output ${PROJECT_BINARY_DIR}/fuzz_input/input${n}.cbor
        --output-as cbor
        )
    endforeach()

    target_sources(fuzz_target PRIVATE fuzz_manifest12.c)
    include(${PROJECT_BINARY_DIR}/manifest12.cmake)
    target_link_libraries(fuzz_target PRIVATE manifest12)

elseif (${TEST_CASE} STREQUAL everything)
    execute_process(
        COMMAND zcbor
        code
        -c ${CMAKE_CURRENT_LIST_DIR}/../cases/everything.cddl
        --output-cmake ${PROJECT_BINARY_DIR}/everything.cmake
        -t EverythingUnion
        -d
        )

    foreach(n RANGE 0 1)
        execute_process(
        COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/../../zcbor/zcbor.py
        convert
        --cddl ${CMAKE_CURRENT_LIST_DIR}/../cases/everything.cddl
        --input ${CMAKE_CURRENT_LIST_DIR}/../cases/everything_example${n}.yaml
        --yaml-compatibility
        -t EverythingUnion
        --output ${PROJECT_BINARY_DIR}/fuzz_input/input${n}.cbor
        --output-as cbor
        )
    endforeach()

    target_sources(fuzz_target PRIVATE fuzz_everything.c)
    include(${PROJECT_BINARY_DIR}/everything.cmake)
    target_link_libraries(fuzz_target PRIVATE everything)

else()
    message(FATAL_ERROR "Invalid test case")
endif()
