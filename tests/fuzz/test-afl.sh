#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

if [ -d "build-afl" ]; then rm -r build-afl; fi
mkdir build-afl
pushd build-afl
cmake .. -DCMAKE_C_COMPILER=afl-clang-fast -DTEST_CASE=manifest12
[[ $? -ne 0 ]] && exit 1
make fuzz_target
[[ $? -ne 0 ]] && exit 1
popd
afl-fuzz -i build-afl/fuzz_input -o build-afl/output -V $1 -- ./build-afl/fuzz_target

