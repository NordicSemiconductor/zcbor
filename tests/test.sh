#!/bin/bash

for dir in 'cbor_decode/' 'cbor_encode/' ;
do
        pushd "$dir"
        ./test.sh
        [[ $? -ne 0 ]] && popd && exit 1
        popd
done
