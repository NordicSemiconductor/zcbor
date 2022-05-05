#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

# This file was used to generate the floating point test cases.
# It's kept here for posterity.

import struct
import numpy
import math
import cbor2

def print_var(val1, val2, bytestr):
	var_str = ""
	for b in bytestr:
		var_str += hex(b)  + ", "
	print(str(val1) + ":", val2, bytestr.hex(), var_str)

def print_32_64(str_val, val = None):
	val = val or float(str_val)
	print_var(str_val, val, struct.pack("!e", numpy.float16(val)))
	print_var(str_val, val, struct.pack("!f", struct.unpack("!e", struct.pack("!e", numpy.float16(val)))[0]))
	print (numpy.float32(struct.unpack("!e", struct.pack("!e", numpy.float16(val)))[0]))
	print_var(str_val, val, struct.pack("!f", val))
	print_var(str_val, val, struct.pack("!d", val))
	print_var(str_val, val, struct.pack("!d", struct.unpack("!f", struct.pack("!f", val))[0]))
	print()

print_32_64("3.1415")
print_32_64("2.71828")
print_32_64("1234567.89")
print_32_64("-98765.4321")
print_32_64("123/456789", 123/456789)
print_32_64("-2^(-42)", -1/(2**(42)))
print_32_64("1.0")
print_32_64("-10000.0")
print_32_64("0.0")
print_32_64("0.000000059604645")
print_32_64("0.000000059604644775390625")
print_32_64("0.00006103515625")
print_32_64("0.000061035153")
print_32_64("65504")
