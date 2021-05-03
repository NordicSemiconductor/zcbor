
add_executable(fuzz_pet
    main_entry.c
    fuzz_pet.c
    generated/pet_decode.c)
target_link_libraries(fuzz_pet PUBLIC cbor_decode)
target_include_directories(fuzz_pet PUBLIC generated)
