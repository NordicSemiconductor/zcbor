#!/bin/bash

for dir in 'test1_suit/' 'test2_simple/' ;
do
        pushd "$dir"
        if [ -d "build" ]; then rm -r build; fi
        mkdir build
        cd build
        cmake -GNinja -DBOARD=native_posix .. 2>&1 1> test.log
        ninja > test.log
        ninja run
        popd
done
