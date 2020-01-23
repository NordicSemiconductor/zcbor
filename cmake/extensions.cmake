#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

# cddl_source(<target> <cddl_file> ENTRY_TYPES <entry_types> [VERBOSE])
#
# Add generated code to the project for decoding CBOR.
#
# <cddl_file> is a file with CDDL data describing the expected CBOR data.
#
# <entry_types> is a list of the types defined in the CDDL file for which there
#               should be an exposed decoding function.
#
# Specify VERBOSE to add printing to the generated code. Check kconfig to add
# printing in the low level CBOR decoding code.
#
# The result of the function is that a library is added to the project which
# contains the generated decoding code. The code is generated at build time using
# a script, and generates a c file and an h file with names derived from the
# name of the <cddl_file>.
function(cddl_source target cddl_file)
  if(IS_ABSOLUTE ${cddl_file})
    set(cddl_path ${cddl_file})
  else()
    set(cddl_path ${CMAKE_CURRENT_SOURCE_DIR}/${cddl_file})
  endif()
  get_filename_component(name ${cddl_path} NAME_WE)
  set(c_file ${PROJECT_BINARY_DIR}/${name}.c)
  set(h_file_dir ${PROJECT_BINARY_DIR}/zephyr/include/generated)
  set(h_file ${h_file_dir}/${name}.h)

  cmake_parse_arguments(CDDL "VERBOSE" "" "ENTRY_TYPES" ${ARGN})
  if (${CDDL_VERBOSE})
    set(verbose_flag "-v")
  endif()

  if (NOT DEFINED PYTHON_EXECUTABLE)
    set(PYTHON_EXECUTABLE python3)
  endif()

  add_custom_command(
    OUTPUT
    ${c_file}
    COMMAND
    ${PYTHON_EXECUTABLE}
    ${CDDL_GEN_BASE}/scripts/cddl_gen.py
    -i ${cddl_path}
    --oc ${c_file}
    --oh ${h_file}
    -t ${CDDL_ENTRY_TYPES}
    ${verbose_flag}
    DEPENDS
    ${cddl_path}
    ${CDDL_GEN_BASE}/scripts/cddl_gen.py
  )
  add_library(${name}_cddl ${c_file})
  target_include_directories(${name}_cddl PUBLIC ${h_file_dir} ${CDDL_GEN_BASE}/include)
  add_custom_target(${name}_c DEPENDS ${c_file}) # wrapper target
  add_dependencies(${name}_cddl ${name}_c)
  target_link_libraries(${name}_cddl PRIVATE cbor_decode cddl_gen_interface)
  target_link_libraries(${target} PRIVATE ${name}_cddl)
  target_include_directories(${target} PRIVATE ${h_file_dir} ${CDDL_GEN_BASE}/include)
endfunction(cddl_source)
