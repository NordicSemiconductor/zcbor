#!/bin/bash
#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

for dir in 'cbor_decode/' 'cbor_encode/' ;
do
        pushd "$dir"
        ./test.sh
        [[ $? -ne 0 ]] && popd && exit 1
        popd
done
