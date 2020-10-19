#!/bin/bash

for dir in 'test1_suit_old_formats/' 'test2_suit/' 'test3_simple/' 'test5_strange/' ;
do
        pushd "$dir"

        if [ -d "build" ]; then rm -r build; fi
        mkdir build &&cd build

        cmake -GNinja -DBOARD=native_posix .. 2>&1 1> test.log
        if [[ $? -ne 0 ]]; then
            cat test.log
            exit 1
        fi

        ninja > test.log
        if [[ $? -ne 0 ]]; then
            cat test.log
            exit 1
        fi

        ninja run
        [[ $? -ne 0 ]] && exit 1

        popd
done
