#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

add_library(cbor_common cbor_common.c)
target_include_directories(cbor_common PUBLIC ${CDDL_GEN_BASE}/include)
target_link_libraries(cbor_common PRIVATE zephyr_interface)

add_library(cbor_decode cbor_decode.c)
target_include_directories(cbor_decode PUBLIC ${CDDL_GEN_BASE}/include)
target_link_libraries(cbor_decode PRIVATE zephyr_interface PUBLIC cbor_common)
