#!/bin/bash
#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#


do_test() {
        test_dir=$1
        test_platform=$2
        extra_args=$3

        pushd "$test_dir"

        if [ -d "build" ]; then rm -r build; fi
        mkdir build &&cd build

        cmake -GNinja -DBOARD=$test_platform .. 2>&1 1> test.log
        if [[ $? -ne 0 ]]; then
            cat test.log
            exit 1
        fi

        ninja >> test.log
        if [[ $? -ne 0 ]]; then
            cat test.log
            exit 1
        fi

        ninja run
        [[ $? -ne 0 ]] && exit 1

        popd
}

do_test_32_64() {
        test_dir=$1
        extra_args=$2

        for board in 'native_posix' 'native_posix_64' ;
        do
                echo ""
                echo "do_test $test_dir $board $extra_args"
                do_test $test_dir $board $extra_args
        done
}


for dir in 'test1_suit_old_formats/' 'test2_suit/' 'test3_simple/' 'test7_suit9_simple' ;
do
        do_test_32_64 $dir ''
done

for canonical in '-DTEST_INDETERMINATE_LENGTH_ARRAYS' '' ;
do
        do_test_32_64 'test5_strange/' $canonical
done
