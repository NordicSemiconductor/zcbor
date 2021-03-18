#!/bin/bash
#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

pycodestyle ../scripts/cddl_gen.py --max-line-length=120 --ignore=W191,E101,W503
[[ $? -ne 0 ]] && exit 1

for dir in 'cbor_decode/' 'cbor_encode/' ;
do
        pushd "$dir"
        ./test.sh
        [[ $? -ne 0 ]] && popd && exit 1
        popd
done
