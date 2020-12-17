#!/bin/bash
#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

do_test() {
        pushd "$1"

        if [ -d "build" ]; then rm -r build; fi
        mkdir build && cd build

        cmake -GNinja -DBOARD=native_posix $2 .. 2>&1 1> test.log
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
}

for dir in 'test1_suit/' 'test2_simple/' 'test3_strange/' ;
do
        do_test $dir -DCANONICAL=CANONICAL
done

for dir in 'test2_simple/' 'test3_strange/' ;
do
        do_test $dir
done
