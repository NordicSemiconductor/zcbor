# Fuzz test
Simple setup using AFL or libFuzzer.

## Libfuzzer
```
$ mkdir build-libfuzzer
$ cd build-libfuzzer
$ cmake ../.. -DCMAKE_C_COMPILER=clang -DCMAKE_C_FLAGS="-fsanitize=fuzzer,address -DLIBFUZZER" -DCMAKE_C_COMPILER_WORKS=On
$ make fuzz_pet
$ ./fuzz-test/fuzz_pet
```

## AFL
```
$ mkdir build-afl
$ cd build-afl
$ export AFL_USE_ASAN=1
$ cmake ../.. -DCMAKE_C_COMPILER=afl-clang-fast
$ make fuzz_pet
```