#!/bin/bash

do_test() {
        pushd "$1"
        if [ -d "build" ]; then rm -r build; fi
        mkdir build
        cd build
        cmake -GNinja -DBOARD=native_posix $2 .. 2>&1 1> test.log
        ninja > test.log
        ninja run
        popd
}

for dir in 'test1_suit/' 'test2_simple/' 'test3_strange/' ;
do
        do_test $dir -DCANONICAL=CANONICAL
done

for dir in 'test2_simple/' 'test3_strange/' ;
do
        do_test $dir
done
