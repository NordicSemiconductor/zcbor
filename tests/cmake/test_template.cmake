
cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(${PROJECT_NAME})

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources}
  )

if (VERBOSE)
  zephyr_compile_definitions(ZCBOR_VERBOSE)
endif()

if (CANONICAL)
  zephyr_compile_definitions(ZCBOR_CANONICAL)
endif()

zephyr_compile_options(-Werror)

if (CONFIG_64BIT)
  set(bit_arg -b 64)
endif()
