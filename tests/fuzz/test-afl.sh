#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

echo "Usage: $0 <seconds to run> <bit width> <test case>"

# For convenience (use for extra checks):
# export AFL_USE_ASAN=1
# For convenience (use if afl fails and recommends this):
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1

if [ -d "build-afl" ]; then rm -r build-afl; fi
mkdir build-afl
pushd build-afl
cmake .. -DCMAKE_C_COMPILER=afl-clang-fast -DCMAKE_C_FLAGS="-m$2" -DTEST_CASE=$3
[[ $? -ne 0 ]] && exit 1
make fuzz_target
[[ $? -ne 0 ]] && exit 1
popd
afl-fuzz -i build-afl/fuzz_input -o build-afl/output -V $1 -- ./build-afl/fuzz_target
exit $(find build-afl/output/default/crashes/ -type f -printf x | wc -c)
