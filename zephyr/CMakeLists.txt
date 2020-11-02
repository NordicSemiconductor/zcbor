if(NOT CONFIG_CDDL_GEN)
  return()
endif()

set(CDDL_GEN_BASE ${CMAKE_CURRENT_LIST_DIR}/.. CACHE FILEPATH
  "Directory of the cddl_gen repo."
  )

include(${ZEPHYR_CURRENT_MODULE_DIR}/cmake/extensions.cmake)

zephyr_library_named(cddl_gen)
zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)

set(src ${ZEPHYR_CURRENT_MODULE_DIR}/src)
zephyr_library_sources(
  ${src}/cbor_common.c
  ${src}/cbor_decode.c
  ${src}/cbor_encode.c
  )
