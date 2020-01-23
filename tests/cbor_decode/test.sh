#!/bin/bash

#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

if [ "$1" == "--help" ] || [ "$1" == "" ] || [ "$2" == "" ]; then
	echo "Run tests."
	echo "Usage: $0 <board> <serial_port>"
	echo "  e.g. $0 nrf52840_pca10056 /dev/ttyACM0"
	echo ""
	echo "Will print SUCCESS or FAIL before exiting when running each test."
	echo "Will return an error code unless SUCCESS."
	exit -1
fi

build_flash() {
	if [ -d "$1" ]; then rm -r $1; fi
	mkdir $1
	cd $1
	cmake -GNinja $2
	ninja
	nrfjprog --recover
	ninja flash
	cd ..
}

print_result_exit_on_error() {
	if [[ $1 == 0 ]]; then
		echo
		echo SUCCESS
		echo
		popd
	else
		echo
		echo FAIL
		echo
		popd
		exit 1
	fi
}

do_test() {
	pushd $1
	echo $1
	echo $2
	echo $3
	build_flash "build" "-DBOARD=$2 .."
	# echo -e "\n" > build/uart.log
	# while read -t 1 l< $3; do
	# 	echo $l >> build/uart.log
	# done
	# cat build/uart.log

	# grep -F "Hello World!" build/uart.log
	# print_result_exit_on_error $?
	popd
}

do_test test1_suit_old_formats $1 $2

do_test test2_suit $1 $2

do_test test3_simple $1 $2

# pushd test4_serial_recovery
# ./test $1 $2
# print_result_exit_on_error $?
# popd
exit 0
