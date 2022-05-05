#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

import numpy
import sys
import os

def decode_test():
	num_start = 0
	num_end = 0x10000

	a = numpy.frombuffer(numpy.arange(num_start, num_end, dtype=numpy.ushort).tobytes(), dtype=numpy.float16)
	with open(os.path.join(sys.argv[1], "fp_bytes_decode.bin"), 'wb') as f:
		f.write(a.astype("<f").tobytes() + a.astype(">f").tobytes())

def encode_test():
	num_start = 0x33000001
	num_end = 0x477ff000

	a = numpy.arange(num_start, num_end, dtype=numpy.uintc)
	b = numpy.frombuffer(a.tobytes(), dtype=numpy.float32).astype("<e") # <e is little endian float16
	c = numpy.where(b[1:] != b[:-1])[0].astype(numpy.uintc) + 1
	assert all(numpy.frombuffer((b[c].tobytes()), dtype=numpy.ushort) == numpy.arange(2, 31744))

	with open(os.path.join(sys.argv[1], "fp_bytes_encode.bin"), 'wb') as f:
		f.write(c.astype("<I").tobytes() + c.astype(">I").tobytes())

def print_help():
	print("Generate bin files with results from converting between float16 and float32 (both ways)")
	print()
	print(f"Usage: {sys.argv[0]} <directory to place bin files in>")

if __name__ == "__main__":
	if "--help" in sys.argv or "-h" in sys.argv or len(sys.argv) < 2:
		print_help()
	elif len(sys.argv) < 3:
		decode_test()
		encode_test()
	elif sys.argv[2] == "decode":
		decode_test()
	elif sys.argv[2] == "encode":
		encode_test()
