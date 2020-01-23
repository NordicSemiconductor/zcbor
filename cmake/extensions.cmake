#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

# target_cddl_source(<target> <cddl_file> ENTRY_TYPES <entry_types> [VERBOSE])
#
# Add generated code to the project for decoding CBOR.
#
# <cddl_file> is a file with CDDL data describing the expected CBOR data.
#
# <entry_types> is a list of the types defined in the CDDL file for which there
#               should be an exposed decoding function.
#
# Specify VERBOSE to print more information while parsing the CDDL, and add
# printing to the generated code and decoding library.
#
# The result of the function is that a library is added to the project which
# contains the generated decoding code. The code is generated at build time
# using a script, and generates a c file and an h file with names derived from
# the name of the <cddl_file>.
function(target_cddl_source target cddl_file)
  if(IS_ABSOLUTE ${cddl_file})
    set(cddl_path ${cddl_file})
  else()
    set(cddl_path ${CMAKE_CURRENT_LIST_DIR}/${cddl_file})
  endif()

  if(NOT EXISTS ${cddl_path})
    message(FATAL_ERROR "CDDL input file ${cddl_file} does not exist.")
  endif()

  get_filename_component(name ${cddl_path} NAME_WE)
  set(c_file ${CMAKE_CURRENT_BINARY_DIR}/${name}.c)
  set(h_file_dir ${CMAKE_CURRENT_BINARY_DIR}/zephyr/include/generated)
  set(h_file ${h_file_dir}/${name}.h)

  cmake_parse_arguments(CDDL "VERBOSE" "" "ENTRY_TYPES" ${ARGN})

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
    $<$<BOOL:${CDDL_VERBOSE}>:-v>
    DEPENDS
    ${cddl_path}
    ${CDDL_GEN_BASE}/scripts/cddl_gen.py
  )

  # Add to provided target
  target_sources(${target} PRIVATE ${c_file})
  target_include_directories(${target}
    PUBLIC
    ${h_file_dir}
    ${CDDL_GEN_BASE}/include
    )
  target_link_libraries(${target} PRIVATE cbor_decode zephyr_interface)
  if (CDDL_VERBOSE)
    target_compile_definitions(cbor_decode PRIVATE CDDL_CBOR_VERBOSE)
    target_compile_definitions(${target} PRIVATE CDDL_CBOR_VERBOSE)
  endif()
endfunction(target_cddl_source)
