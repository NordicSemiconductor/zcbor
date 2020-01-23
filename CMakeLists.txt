#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

set(CDDL_GEN_BASE ${CMAKE_CURRENT_LIST_DIR} CACHE FILEPATH
  "Directory of the cddl_gen repo."
  )
message("CDDL_GEN_BASE: ${CDDL_GEN_BASE}")

include(cmake/extensions.cmake)
add_subdirectory(src)

add_library(cddl_gen_interface INTERFACE)
target_include_directories(cddl_gen_interface INTERFACE ${CDDL_GEN_BASE}/include)

# Currently needed to build tests.
target_link_libraries(cddl_gen_interface INTERFACE zephyr_interface)
