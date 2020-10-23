#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

# generate_cddl(<cddl_file>
#               ENTRY_TYPES <entry_types>
#               C_FILE <c_file_path>
#               H_FILE <h_file_path>
#               DECODE|ENCODE
#               [VERBOSE]
#               [TYPE_FILE <type_file_path>])
#
# Add generated code to the project for decoding CBOR.
#
# REQUIRED arguments:
# <cddl_file> is a file with CDDL data describing the expected CBOR data.
#
# <entry_types> is a list of the types defined in the CDDL file for which there
#               should be an exposed decoding function.
#
# <c_file_path> is the path at which to place the generated c file.
#
# <h_file_path> is the path at which to place the generated h file.
#
# OPTIONAL arguments:
# Specify either DECODE or ENCODE, to generate code for decoding or encoding,
# respectively. If neither is provided, default to DECODE.
#
# Specify VERBOSE to print more information while parsing the CDDL, and add
# printing to the generated code and decoding library.
#
# If provided, <type_file_path> overrides the default typedef file path.
#
# The result of the function is that code is generated at build time
# using a script, generating a c file and an h file (and a typedef file) with
# names derived from the name of the <cddl_file>.
function(generate_cddl cddl_file)
  if(IS_ABSOLUTE ${cddl_file})
    set(cddl_path ${cddl_file})
  else()
    set(cddl_path ${CMAKE_CURRENT_LIST_DIR}/${cddl_file})
  endif()

  if(NOT EXISTS ${cddl_path})
    message(FATAL_ERROR "CDDL input file ${cddl_file} does not exist.")
  endif()

  cmake_parse_arguments(CDDL
    "DECODE;ENCODE;VERBOSE"
    "C_FILE;H_FILE;TYPE_FILE"
    "ENTRY_TYPES"
    ${ARGN}
    )

  if ((NOT CDDL_ENTRY_TYPES) OR (NOT CDDL_C_FILE) OR (NOT CDDL_H_FILE) OR (CDDL_ENCODE AND CDDL_DECODE))
    message(FATAL_ERROR "Missing arguments or illegal combination of arguments.")
  endif()

  if (DEFINED CDDL_TYPE_FILE)
    set(type_file_arg --oht ${CDDL_TYPE_FILE})
  endif()

  if ("${CDDL_DECODE}" STREQUAL "${CDDL_ENCODE}")
    message(FATAL_ERROR "Please specify exactly one of DECODE or ENCODE")
  endif()

  add_custom_command(
    OUTPUT
    ${CDDL_C_FILE}
    ${CDDL_H_FILE}
    COMMAND
    ${PYTHON_EXECUTABLE}
    ${CDDL_GEN_BASE}/scripts/cddl_gen.py
    -i ${cddl_path}
    --oc ${CDDL_C_FILE}
    --oh ${CDDL_H_FILE}
    ${type_file_arg}
    -t ${CDDL_ENTRY_TYPES}
    $<$<BOOL:${CDDL_DECODE}>:-d>
    $<$<BOOL:${CDDL_ENCODE}>:-e>
    $<$<BOOL:${CDDL_VERBOSE}>:-v>
    DEPENDS
    ${cddl_path}
    ${CDDL_GEN_BASE}/scripts/cddl_gen.py
  )
endfunction()




# target_cddl_source(<target>
#                    <cddl_file>
#                    ENTRY_TYPES <entry_types>
#                    DECODE|ENCODE
#                    [VERBOSE] [CANONICAL]
#                    [TYPE_FILE_NAME <filename>])
#
# Add generated code to the project for decoding CBOR.
#
# REQUIRED arguments:
# <cddl_file> is a file with CDDL data describing the expected CBOR data.
#
# <entry_types> is a list of the types defined in the CDDL file for which there
#               should be an exposed decoding function.
#
# OPTIONAL arguments:
# Specify either DECODE or ENCODE, to generate code for decoding or encoding,
# respectively. If neither is provided, default to DECODE.
#
# Specify VERBOSE to print more information while parsing the CDDL, and add
# printing to the generated code and decoding library.
#
# Specify CANONICAL to make the encoder generate canonical CBOR. This can make
# code size slightly bigger. This option has no effect together with DECODE.
#
# If provided, TYPE_FILE_NAME can be used to specify what the file containing
# typedefs should be called.
#
# The result of the function is that a library is added to the project which
# contains the generated decoding code. The code is generated at build time
# using a script, and generates a c file and an h file (and typedef file) with
# names derived from the name of the <cddl_file>.
function(target_cddl_source target cddl_file)
  if(IS_ABSOLUTE ${cddl_file})
    set(cddl_path ${cddl_file})
  else()
    set(cddl_path ${CMAKE_CURRENT_LIST_DIR}/${cddl_file})
  endif()

  cmake_parse_arguments(CDDL
    "DECODE;ENCODE;VERBOSE;CANONICAL"
    "TYPE_FILE_NAME"
    "ENTRY_TYPES"
    ${ARGN}
    )

  if ((NOT CDDL_ENTRY_TYPES) OR (CDDL_ENCODE AND CDDL_DECODE))
    message(FATAL_ERROR "Missing arguments or illegal combination of arguments.")
  endif()

  if ("${CDDL_DECODE}" STREQUAL "${CDDL_ENCODE}")
    message(FATAL_ERROR "Please specify exactly one of DECODE or ENCODE")
  endif()

  if (CDDL_ENCODE)
    set(code "encode")
    set(CODE "ENCODE")
  else ()
    set(code "decode")
    set(CODE "DECODE")
  endif()

  get_filename_component(name ${cddl_path} NAME_WE)
  set(c_file ${CMAKE_CURRENT_BINARY_DIR}/${name}_${code}.c)
  set(h_file_dir ${CMAKE_CURRENT_BINARY_DIR}/zephyr/include/generated)
  set(h_file ${h_file_dir}/${name}_${code}.h)

  if (DEFINED CDDL_TYPE_FILE_NAME)
    set(type_file_path ${h_file_dir}/${CDDL_TYPE_FILE_NAME})
  endif()

  generate_cddl(${cddl_file} ${CODE} ${CDDL_VERBOSE} ${CDDL_CANONICAL}
    ENTRY_TYPES ${CDDL_ENTRY_TYPES}
    C_FILE ${c_file} H_FILE ${h_file} TYPE_FILE ${type_file_path}
    )

  # Add to provided target
  target_sources(${target} PRIVATE ${c_file})
  target_include_directories(${target}
    PUBLIC
    ${h_file_dir}
    ${CDDL_GEN_BASE}/include
    )

  if (${CDDL_DECODE})
    set(cbor_lib cbor_decode)
  else()
    set(cbor_lib cbor_encode)
  endif()
  target_link_libraries(${target} PRIVATE ${cbor_lib} zephyr_interface)
  if (CDDL_VERBOSE)
    target_compile_definitions(${cbor_lib} PRIVATE CDDL_CBOR_VERBOSE)
    target_compile_definitions(${target} PRIVATE CDDL_CBOR_VERBOSE)
    target_compile_definitions(cbor_common PRIVATE CDDL_CBOR_VERBOSE)
  endif()
  if (CDDL_CANONICAL)
    target_compile_definitions(${cbor_lib} PRIVATE CDDL_CBOR_CANONICAL)
    target_compile_definitions(${target} PRIVATE CDDL_CBOR_CANONICAL)
    target_compile_definitions(cbor_common PRIVATE CDDL_CBOR_CANONICAL)
  endif()
endfunction(target_cddl_source)
