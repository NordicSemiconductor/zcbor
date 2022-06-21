FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources}
  )

if (VERBOSE)
  zephyr_compile_definitions(ZCBOR_VERBOSE)
endif()

if (ASSERTS)
  zephyr_compile_definitions(ZCBOR_ASSERTS)
endif()

if (CANONICAL)
  zephyr_compile_definitions(ZCBOR_CANONICAL)
endif()

zephyr_compile_options(-Werror)

if (CONFIG_64BIT)
  set(bit_arg -b 64)
endif()
