#!/bin/bash

for dir in 'cbor_decode/' 'cbor_encode/' ;
do
        pushd "$dir"
        ./test.sh
        popd
done
