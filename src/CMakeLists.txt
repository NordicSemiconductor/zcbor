#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

add_library(cbor_decode cbor_decode.c)
target_include_directories(cbor_decode PUBLIC ${CDDL_GEN_BASE}/include)
target_link_libraries(cbor_decode PRIVATE zephyr_interface)
