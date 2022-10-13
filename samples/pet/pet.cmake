#
# Generated using zcbor version 0.6.99
# https://github.com/NordicSemiconductor/zcbor
#

add_library(pet)
target_sources(pet PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../src/zcbor_decode.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/zcbor_encode.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/zcbor_common.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pet_decode.c
    ${CMAKE_CURRENT_LIST_DIR}/src/pet_encode.c
    )
target_include_directories(pet PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../../include
    ${CMAKE_CURRENT_LIST_DIR}/include
    )
