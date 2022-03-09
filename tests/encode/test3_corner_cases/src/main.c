/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include "corner_cases_encode.h"
#include "zcbor_debug.h" // Enables use of print functions when debugging tests.

#define CONCAT_BYTE(a,b) a ## b

#ifndef ZCBOR_CANONICAL
#define LIST(num) 0x9F
#define MAP(num) 0xBF
#define END 0xFF,
#else
#define LIST(num) CONCAT_BYTE(0x8, num)
#define MAP(num) CONCAT_BYTE(0xA, num)
#define END
#endif


void test_numbers(void)
{
	const uint8_t exp_payload_numbers1[] = {
		LIST(A), // List start
			0x01, // 1
			0x21, // -2
			0x05, // 5
			0x19, 0x01, 0x00, // 256
			0x1A, 0x01, 0x02, 0x03, 0x04, // 0x01020304
			0x39, 0x13, 0x87, // -5000
			0x1A, 0xEE, 0x6B, 0x28, 0x00, // 4000000000
			0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -2^31
			0x00, // 0
			0xD9, 0xFF, 0xFF, 0x01, // 1
		END
	};

	struct Numbers numbers = {0};
	uint8_t output[100];
	size_t out_len;

	numbers._Numbers_fourtoten = 3; // Invalid
	numbers._Numbers_twobytes = 256;
	numbers._Numbers_onetofourbytes = 0x01020304;
	numbers._Numbers_minusfivektoplustwohundred = -5000;
	numbers._Numbers_negint = -2147483648;
	numbers._Numbers_posint = 0;
	numbers._Numbers_tagged_int = 1;

	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_Numbers(output,
		sizeof(output), &numbers, &out_len), NULL);

	numbers._Numbers_fourtoten = 11; // Invalid
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_Numbers(output,
		sizeof(output), &numbers, &out_len), NULL);

	numbers._Numbers_fourtoten = 5; // Valid
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Numbers(output,
		sizeof(output), &numbers, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_numbers1), out_len, "%d != %d\r\n", sizeof(exp_payload_numbers1), out_len);
	zassert_mem_equal(exp_payload_numbers1, output, sizeof(exp_payload_numbers1), NULL);

	numbers._Numbers_negint = 1; // Invalid
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_Numbers(output,
		sizeof(output), &numbers, &out_len), NULL);

	numbers._Numbers_negint = -1; // Valid
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Numbers(output,
		sizeof(output), &numbers, &out_len), NULL);

	numbers._Numbers_minusfivektoplustwohundred = -5001; // Invalid
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_Numbers(output,
		sizeof(output), &numbers, &out_len), NULL);

	numbers._Numbers_minusfivektoplustwohundred = 201; // Invalid
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_Numbers(output,
		sizeof(output), &numbers, &out_len), NULL);

	numbers._Numbers_minusfivektoplustwohundred = 200; // Valid
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Numbers(output,
		sizeof(output), &numbers, &out_len), NULL);
}

void test_numbers2(void)
{
	const uint8_t exp_payload_numbers2[] = {
		LIST(6),
			0x1A, 0x00, 0x12, 0x34, 0x56, // 0x123456
			0x1B, 0x01, 2, 3, 4, 5, 6, 7, 8, // 0x0102030405060708
			0x1B, 0x11, 2, 3, 4, 5, 6, 7, 9, // 0x1102030405060709
			0x00, // 0
			0x3A, 0x80, 0x00, 0x00, 0x00, // -0x8000_0001
			0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -0x8000_0000
		END
	};
	struct Numbers2 numbers2 = {
		._Numbers2_threebytes = 0x123456,
		._Numbers2_big_int = 0x0102030405060708,
		._Numbers2_big_uint = 0x1102030405060709,
		._Numbers2_big_uint2 = 0,
	};
	uint8_t output[100];
	size_t out_len;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Numbers2(output,
		sizeof(output), &numbers2, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_numbers2), out_len, "%d != %d\r\n",
		sizeof(exp_payload_numbers2), out_len);
	zassert_mem_equal(exp_payload_numbers2, output, sizeof(exp_payload_numbers2), NULL);
}

void test_number_map(void)
{
	size_t encode_len = 0xFFFFFFFF;
	const uint8_t exp_payload_number_map1[] = {
		MAP(3),
			0x64, 'b', 'y', 't', 'e',
			0x18, 42,
			0x69, 'o', 'p', 't', '_', 's', 'h', 'o', 'r', 't',
			0x19, 0x12, 0x34,
			0x68, 'o', 'p', 't', '_', 'c', 'b', 'o', 'r',
			0x45, 0x1A, 0x12, 0x34, 0x56, 0x78,
		END
	};

	const uint8_t exp_payload_number_map2[] = {
		MAP(2),
			0x64, 'b', 'y', 't', 'e',
			0x18, 42,
			0x68, 'o', 'p', 't', '_', 'c', 'b', 'o', 'r',
			0x45, 0x1A, 0x12, 0x34, 0x56, 0x78,
		END
	};

	uint8_t payload[80];

	struct NumberMap number_map1 = {
		._NumberMap_byte = 42,
		._NumberMap_opt_short_present = true,
		._NumberMap_opt_short._NumberMap_opt_short = 0x1234,
		._NumberMap_opt_cbor_present = true,
		._NumberMap_opt_cbor._NumberMap_opt_cbor_cbor = 0x12345678,
	};

	int res = cbor_encode_NumberMap(payload,
		sizeof(payload), &number_map1, &encode_len);
	zassert_equal(ZCBOR_SUCCESS, res, "%d\r\n", res);
	zassert_equal(encode_len, sizeof(exp_payload_number_map1), NULL);
	zassert_mem_equal(payload, exp_payload_number_map1, encode_len, NULL);

	struct NumberMap number_map2 = {
		._NumberMap_byte = 42,
		._NumberMap_opt_short_present = false,
		._NumberMap_opt_cbor_present = true,
		._NumberMap_opt_cbor._NumberMap_opt_cbor_cbor = 0x12345678,
	};

	res = cbor_encode_NumberMap(payload,
		sizeof(payload), &number_map2, &encode_len);
	zassert_equal(ZCBOR_SUCCESS, res, "%d\r\n", res);
	zassert_equal(encode_len, sizeof(exp_payload_number_map2), NULL);
	zassert_mem_equal(payload, exp_payload_number_map2, encode_len, NULL);

	struct NumberMap number_map3 = {
		._NumberMap_byte = 42,
		._NumberMap_opt_short_present = false,
		._NumberMap_opt_cbor_present = true,
		._NumberMap_opt_cbor._NumberMap_opt_cbor.value = (uint8_t[]){0x1A, 0x12, 0x34, 0x56, 0x78},
		._NumberMap_opt_cbor._NumberMap_opt_cbor.len = 5,
	};

	res = cbor_encode_NumberMap(payload,
		sizeof(payload), &number_map3, &encode_len);
	zassert_equal(ZCBOR_SUCCESS, res, "%d\r\n", res);
	zassert_equal(encode_len, sizeof(exp_payload_number_map2), NULL);
	zassert_mem_equal(payload, exp_payload_number_map2, encode_len, NULL);

	struct NumberMap number_map4_inv = {
		._NumberMap_byte = 42,
		._NumberMap_opt_short_present = false,
		._NumberMap_opt_cbor_present = true,
		._NumberMap_opt_cbor._NumberMap_opt_cbor.value = (uint8_t[]){0x19, 0x12, 0x34},
		._NumberMap_opt_cbor._NumberMap_opt_cbor.len = 3,
	};

	res = cbor_encode_NumberMap(payload,
		sizeof(payload), &number_map4_inv, &encode_len);
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, res, "%d\r\n", res);
}


void test_strings(void)
{
	const uint8_t exp_payload_strings1[] = {
		LIST(6),
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x59, 0x01, 0x2c, // 300 bytes
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0xC0, 0x78, 0x1E, // 30 bytes
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
#ifndef ZCBOR_CANONICAL
		0x58, 30, // Numbers (len: 30)
#else
		0x58, 29, // Numbers (len: 29)
#endif
			LIST(A), // List start
				0x01, // 1
				0x21, // -2
				0x05, // 5
				0x19, 0xFF, 0xFF, // 0xFFFF
				0x18, 0x18, // 24
				0x00, // 0
				0x1A, 0xEE, 0x6B, 0x28, 0x00, // 4000000000
				0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -2^31
				0x1A, 0xFF, 0xFF, 0xFF, 0xFF, // 0xFFFFFFFF
				0xD9, 0xFF, 0xFF, 9, // 9
				END
#ifndef ZCBOR_CANONICAL
		0x55, // Primitives (len: 21)
#else
		0x52, // Primitives (len: 18)
#endif
			LIST(5), // List start
				0xF5, // True
				0xF4, // False
				0xF4, // False
				0xF6, // Nil
				0xF7, // Undefined
				END
			LIST(5), // List start
				0xF5, // True
				0xF4, // False
				0xF5, // True
				0xF6, // Nil
				0xF7, // Undefined
				END
			LIST(5), // List start
				0xF5, // True
				0xF4, // False
				0xF4, // False
				0xF6, // Nil
				0xF7, // Undefined
				END
#ifndef ZCBOR_CANONICAL
		0x59, 0x01, 0x6B, // Strings (len: 363)
#else
		0x59, 0x01, 0x68, // Strings (len: 360)
#endif
			LIST(5),
			0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
			0x59, 0x01, 0x2c, // 300 bytes
			0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
			0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
			0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
			0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
			0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
			0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
			0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
			0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
			0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
			0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
			0xC0, 0x6A, // 10 bytes
			0,1,2,3,4,5,6,7,8,9,
#ifndef ZCBOR_CANONICAL
			0x58, 30, // Numbers (len: 30)
#else
			0x58, 29, // Numbers (len: 29)
#endif
				LIST(A), // List start
					0x01, // 1
					0x21, // -2
					0x05, // 5
					0x19, 0xFF, 0xFF, // 0xFFFF
					0x18, 0x18, // 24
					0x00, // 0
					0x1A, 0xEE, 0x6B, 0x28, 0x00, // 4000000000
					0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -2^31
					0x1A, 0xFF, 0xFF, 0xFF, 0xFF, // 0xFFFFFFFF
					0xD9, 0xFF, 0xFF, 0x29, // -10
					END
#ifndef ZCBOR_CANONICAL
			0x47, // Primitives (len: 7)
#else
			0x46, // Primitives (len: 6)
#endif
				LIST(5), // List start
					0xF5, // True
					0xF4, // False
					0xF4, // False
					0xF6, // Nil
					0xF7, // Undefined
					END
			END
		END
	};

	const uint8_t bytes300[] = {
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
	};

	struct Strings strings1 = {0};
	struct Strings strings2 = {0};
	struct Numbers numbers1 = {0};
	uint8_t output1[100];
	uint8_t output2[100];
	uint8_t output3[400];
	uint8_t output4[800];
	size_t out_len;

	numbers1._Numbers_fourtoten = 5;
	numbers1._Numbers_twobytes = 0xFFFF;
	numbers1._Numbers_onetofourbytes = 24;
	numbers1._Numbers_minusfivektoplustwohundred = 0;
	numbers1._Numbers_negint = -2147483648;
	numbers1._Numbers_posint = 0xFFFFFFFF;
	numbers1._Numbers_tagged_int = 9;

	strings1._Strings_optCborStrings_present = true;
	strings1._Strings_threehundrebytebstr.len = 300;
	strings1._Strings_threehundrebytebstr.value = bytes300;
	strings1._Strings_tentothirtybytetstr.len = 30;
	strings1._Strings_tentothirtybytetstr.value = bytes300;
	strings1._Strings_cborseqPrimitives_cbor_count = 3;
	strings1._Strings_cborseqPrimitives_cbor[0]._Primitives_boolval = false;
	strings1._Strings_cborseqPrimitives_cbor[1]._Primitives_boolval = true;
	strings1._Strings_cborseqPrimitives_cbor[2]._Primitives_boolval = false;

	strings2._Strings_threehundrebytebstr.len = 300;
	strings2._Strings_threehundrebytebstr.value = bytes300;
	strings2._Strings_tentothirtybytetstr.len = 9; // Invalid
	strings2._Strings_tentothirtybytetstr.value = bytes300;
	strings2._Strings_cborseqPrimitives_cbor_count = 1;
	strings2._Strings_cborseqPrimitives_cbor[0]._Primitives_boolval = false;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Numbers(output2, sizeof(output2), &numbers1, &out_len), NULL);
	strings1._Strings_cborNumbers.value = output2;
	strings1._Strings_cborNumbers.len = out_len;
	numbers1._Numbers_tagged_int = -10;
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Numbers(output1, sizeof(output1), &numbers1, &out_len), NULL);
	strings2._Strings_cborNumbers.value = output1;
	strings2._Strings_cborNumbers.len = out_len;
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_Strings(output3, sizeof(output3), &strings2, &out_len), NULL);
	strings2._Strings_tentothirtybytetstr.len = 31; // Invalid
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_Strings(output3, sizeof(output3), &strings2, &out_len), NULL);
	strings2._Strings_tentothirtybytetstr.len = 10; // Valid
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Strings(output3, sizeof(output3), &strings2, &out_len), NULL);
	strings1._Strings_optCborStrings.value = output3;
	strings1._Strings_optCborStrings.len = out_len;
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Strings(output4, sizeof(output4), &strings1, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_strings1), out_len, "expected: %d, actual: %d\r\n", sizeof(exp_payload_strings1), out_len);

	zassert_mem_equal(exp_payload_strings1, output4, sizeof(exp_payload_strings1), NULL);
}

void test_primitives(void)
{
	uint8_t exp_payload_prim1[] = {LIST(5), 0xF5, 0xF4, 0xF4, 0xF6, 0xF7, END};
	uint8_t exp_payload_prim2[] = {LIST(5), 0xF5, 0xF4, 0xF5, 0xF6, 0xF7, END};

	struct Primitives input;
	size_t len_encode;
	uint8_t output[10];

	input._Primitives_boolval = false;
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Prim2(output, sizeof(output), &input, &len_encode), NULL);
	zassert_equal(len_encode, sizeof(exp_payload_prim1), NULL);
	zassert_mem_equal(exp_payload_prim1, output, sizeof(exp_payload_prim1), NULL);

	input._Primitives_boolval = true;
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Prim2(output, sizeof(output), &input, &len_encode), NULL);
	zassert_equal(len_encode, sizeof(exp_payload_prim2), NULL);
	zassert_mem_equal(exp_payload_prim2, output, sizeof(exp_payload_prim2), NULL);
}

void test_optional(void)
{
	const uint8_t exp_payload_optional1[] = {
		LIST(3) /* List start */, 0xCA /* tag */, 0xF4 /* False */, 0x02, 0x03, END
	};
	const uint8_t exp_payload_optional2[] = {
		LIST(2) /* List start */, 0xCA /* tag */, 0xF4 /* False */, 0x03, END
	};
	const uint8_t exp_payload_optional3[] = {
		LIST(3) /* List start */, 0xCA /* tag */, 0xF4 /* False */, 0x02, 0x01, END
	};
	const uint8_t exp_payload_optional4[] = {
		LIST(3) /* List start */, 0xCA /* tag */, 0xF5 /* True */, 0x02, 0x02, END
	};
	const uint8_t exp_payload_optional5[] = {
		LIST(4) /* List start */, 0xCA /* tag */, 0xF5 /* True */, 0xF4 /* False */, 0x02, 0x02, END
	};
	const uint8_t exp_payload_optional6[] = {
		LIST(5) /* List start */, 0xCA /* tag */, 0xF5 /* True */, 0xF4 /* False */, 0x02, 0x02, 0x08, END
	};
	const uint8_t exp_payload_optional7[] = {
		LIST(7) /* List start */, 0xCA /* tag */, 0xF5 /* True */, 0xF4 /* False */, 0x02, 0x02, 0x08, 0x08, 0x08, END
	};

	struct Optional optional1 = {._Optional_opttwo_present = true, ._Optional_manduint = 3};
	struct Optional optional2 = {._Optional_manduint = 3};
	struct Optional optional3 = {._Optional_opttwo_present = true, ._Optional_manduint = 1};
	struct Optional optional4 = {._Optional_boolval = true, ._Optional_opttwo_present = true,
				._Optional_manduint = 2};
	struct Optional optional5 = {._Optional_boolval = true, ._Optional_optbool_present = true,
				._Optional_opttwo_present = true, ._Optional_manduint = 2};
	struct Optional optional6 = {._Optional_boolval = true, ._Optional_optbool_present = true,
				._Optional_opttwo_present = true, ._Optional_manduint = 2,
				._Optional_multi8_count = 1};
	struct Optional optional7 = {._Optional_boolval = true, ._Optional_optbool_present = true,
				._Optional_opttwo_present = true, ._Optional_manduint = 2,
				._Optional_multi8_count = 3};
	uint8_t output[10];
	size_t out_len;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Optional(output,
			sizeof(output), &optional1, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_optional1), out_len, NULL);
	zassert_mem_equal(exp_payload_optional1, output, sizeof(exp_payload_optional1), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Optional(output,
			sizeof(output), &optional2, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_optional2), out_len, NULL);
	zassert_mem_equal(exp_payload_optional2, output, sizeof(exp_payload_optional2), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Optional(output,
			sizeof(output), &optional3, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_optional3), out_len, NULL);
	zassert_mem_equal(exp_payload_optional3, output, sizeof(exp_payload_optional3), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Optional(output,
			sizeof(output), &optional4, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_optional4), out_len, NULL);
	zassert_mem_equal(exp_payload_optional4, output, sizeof(exp_payload_optional4), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Optional(output,
			sizeof(output), &optional5, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_optional5), out_len, NULL);
	zassert_mem_equal(exp_payload_optional5, output, sizeof(exp_payload_optional5), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Optional(output,
			sizeof(output), &optional6, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_optional6), out_len, NULL);
	zassert_mem_equal(exp_payload_optional6, output, sizeof(exp_payload_optional6), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Optional(output,
			sizeof(output), &optional7, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_optional7), out_len, NULL);
	zassert_mem_equal(exp_payload_optional7, output, sizeof(exp_payload_optional7), NULL);
}

void test_union(void)
{
	const uint8_t exp_payload_union1[] = {0x01, 0x21};
	const uint8_t exp_payload_union2[] = {0x03, 0x23};
	const uint8_t exp_payload_union3[] = {0x03, 0x04};
	const uint8_t exp_payload_union4[] = {
		0x67, 0x22, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x22 // "\"hello\""
	};
	const uint8_t exp_payload_union5[] = {
		0x03, 0x23, 0x03, 0x23, 0x03, 0x23,
		0x03, 0x23, 0x03, 0x23, 0x03, 0x23
	};

	struct Union_ _union1 = {._Union_choice = _Union__Group};
	struct Union_ _union2 = {._Union_choice = _Union__MultiGroup, ._Union__MultiGroup._MultiGroup_count = 1};
	struct Union_ _union3 = {._Union_choice = _Union__uint3};
	struct Union_ _union4 = {._Union_choice = _Union_hello_tstr};
	struct Union_ _union5 = {._Union_choice = _Union__MultiGroup, ._Union__MultiGroup._MultiGroup_count = 6};
	struct Union_ _union6_inv = {._Union_choice = _Union__MultiGroup, ._Union__MultiGroup._MultiGroup_count = 7};

	uint8_t output[15];
	size_t out_len;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Union(output, sizeof(output),
				&_union1, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_union1), out_len, NULL);
	zassert_mem_equal(exp_payload_union1, output, sizeof(exp_payload_union1), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Union(output, sizeof(output),
				&_union2, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_union2), out_len, NULL);
	zassert_mem_equal(exp_payload_union2, output, sizeof(exp_payload_union2), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Union(output, sizeof(output),
				&_union3, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_union3), out_len, NULL);
	zassert_mem_equal(exp_payload_union3, output, sizeof(exp_payload_union3), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Union(output, sizeof(output),
				&_union4, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_union4), out_len, NULL);
	zassert_mem_equal(exp_payload_union4, output, sizeof(exp_payload_union4), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Union(output, sizeof(output),
				&_union5, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_union5), out_len, NULL);
	zassert_mem_equal(exp_payload_union5, output, sizeof(exp_payload_union5), NULL);

	zassert_equal(ZCBOR_ERR_ITERATIONS, cbor_encode_Union(output, sizeof(output),
				&_union6_inv, &out_len), NULL);
}

void test_levels(void)
{
	const uint8_t exp_payload_levels1[] = {
		LIST(1), // Level1
		LIST(2), // Level2
		LIST(4), // Level3 no 1
		LIST(1), 0x00, END // Level4 no 1
		LIST(1), 0x00, END // Level4 no 2
		LIST(1), 0x00, END // Level4 no 3
		LIST(1), 0x00, END // Level4 no 4
		END
		LIST(4), // Level3 no 2
		LIST(1), 0x00, END // Level4 no 1
		LIST(1), 0x00, END // Level4 no 2
		LIST(1), 0x00, END // Level4 no 3
		LIST(1), 0x00, END // Level4 no 4
		END END END
	};
	uint8_t output[32];
	size_t out_len;

	struct Level2 level1 = {._Level2__Level3_count = 2, ._Level2__Level3 = {
		{._Level3__Level4_count = 4}, {._Level3__Level4_count = 4}
	}};
	_Static_assert(sizeof(exp_payload_levels1) <= sizeof(output),
		"Payload is larger than output");
	zassert_equal(ZCBOR_ERR_NO_PAYLOAD, cbor_encode_Level1(output,
		sizeof(exp_payload_levels1)-1, &level1, &out_len), NULL);
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Level1(output,
		sizeof(output), &level1, &out_len), NULL);

	zassert_equal(sizeof(exp_payload_levels1), out_len, "%d != %d", sizeof(exp_payload_levels1), out_len);
	zassert_mem_equal(exp_payload_levels1, output, sizeof(exp_payload_levels1), NULL);
}


void test_map(void)
{
	const uint8_t exp_payload_map1[] = {
		MAP(4), LIST(2), 0x05, 0x06, END 0xF4, // [5,6] => false
		0x07, 0x01, // 7 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
		0xf6, 0x40, // nil => ""
		END
	};
	const uint8_t exp_payload_map2[] = {
		MAP(5), LIST(2), 0x05, 0x06, END 0xF5, // [5,6] => true
		0x07, 0x01, // 7 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
		0xf6, 0x40, // nil => ""
		0xf6, 0x40, // nil => ""
		END
	};
	const uint8_t exp_payload_map3[] = {
		MAP(4), LIST(2), 0x05, 0x06, END 0xF4, // [5,6] => false
		0x27, 0x01, // -8 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
		0xf6, 0x40, // nil => ""
		END
	};

	struct Map map1 = {
		._Map_union_choice = _union_uint7uint,
		._union_uint7uint = 1,
		._Map_twotothree_count = 2,
		._Map_twotothree = {
			{._Map_twotothree = {.value = "hello", .len = 5}},
			{._Map_twotothree = {.len = 0}},
		}
	};
	struct Map map2 = {
		._Map_key = true,
		._Map_union_choice = _union_uint7uint,
		._union_uint7uint = 1,
		._Map_twotothree_count = 3,
		._Map_twotothree = {
			{._Map_twotothree = {.value = "hello", .len = 5}},
			{._Map_twotothree = {.len = 0}},
			{._Map_twotothree = {.len = 0}},
		}
	};
	struct Map map3 = {
		._Map_union_choice = _union_nintuint,
		._union_nintuint = 1,
		._Map_twotothree_count = 2,
		._Map_twotothree = {
			{._Map_twotothree = {.value = "hello", .len = 5}},
			{._Map_twotothree = {.len = 0}},
		}
	};

	uint8_t output[25];
	size_t out_len;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Map(output, sizeof(output),
			&map1, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_map1), out_len, NULL);
	zassert_mem_equal(exp_payload_map1, output, sizeof(exp_payload_map1), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Map(output, sizeof(output),
			&map2, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_map2), out_len, NULL);
	zassert_mem_equal(exp_payload_map2, output, sizeof(exp_payload_map2), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Map(output, sizeof(output),
			&map3, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_map3), out_len, NULL);
	zassert_mem_equal(exp_payload_map3, output, sizeof(exp_payload_map3), NULL);
}

void test_nested_list_map(void)
{
	const uint8_t exp_payload_nested_lm1[] = {LIST(0), END};
	const uint8_t exp_payload_nested_lm2[] = {LIST(1), MAP(0), END END};
	const uint8_t exp_payload_nested_lm3[] = {LIST(1), MAP(1), 0x01, 0x04, END END};
	const uint8_t exp_payload_nested_lm4[] = {LIST(2), MAP(0), END MAP(1), 0x01, 0x04, END END};
	const uint8_t exp_payload_nested_lm5[] = {LIST(3), MAP(0), END MAP(0), END MAP(0), END END};
	struct NestedListMap listmap1 = {
		._NestedListMap_map_count = 0,
	};
	struct NestedListMap listmap2 = {
		._NestedListMap_map_count = 1,
		._NestedListMap_map = {
			{._map_uint4_present = false},
		}
	};
	struct NestedListMap listmap3 = {
		._NestedListMap_map_count = 1,
		._NestedListMap_map = {
			{._map_uint4_present = true},
		}
	};
	struct NestedListMap listmap4 = {
		._NestedListMap_map_count = 2,
		._NestedListMap_map = {
			{._map_uint4_present = false},
			{._map_uint4_present = true},
		}
	};
	struct NestedListMap listmap5 = {
		._NestedListMap_map_count = 3,
		._NestedListMap_map = {
			{._map_uint4_present = false},
			{._map_uint4_present = false},
			{._map_uint4_present = false},
		}
	};
	uint8_t output[40];
	size_t out_len;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_NestedListMap(output,
			sizeof(output), &listmap1, &out_len), NULL);

	zassert_equal(sizeof(exp_payload_nested_lm1), out_len, NULL);
	zassert_mem_equal(exp_payload_nested_lm1, output, sizeof(exp_payload_nested_lm1), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_NestedListMap(output,
			sizeof(output), &listmap2, &out_len), NULL);

	zassert_equal(sizeof(exp_payload_nested_lm2), out_len, NULL);
	zassert_mem_equal(exp_payload_nested_lm2, output, sizeof(exp_payload_nested_lm2), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_NestedListMap(output,
			sizeof(output), &listmap3, &out_len), NULL);

	zassert_equal(sizeof(exp_payload_nested_lm3), out_len, NULL);
	zassert_mem_equal(exp_payload_nested_lm3, output, sizeof(exp_payload_nested_lm3), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_NestedListMap(output,
			sizeof(output), &listmap4, &out_len), NULL);

	zassert_equal(sizeof(exp_payload_nested_lm4), out_len, NULL);
	zassert_mem_equal(exp_payload_nested_lm4, output, sizeof(exp_payload_nested_lm4), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_NestedListMap(output,
			sizeof(output), &listmap5, &out_len), NULL);

	zassert_equal(sizeof(exp_payload_nested_lm5), out_len, "%d != %d", sizeof(exp_payload_nested_lm5), out_len);
	zassert_mem_equal(exp_payload_nested_lm5, output, sizeof(exp_payload_nested_lm5), NULL);
}

void test_nested_map_list_map(void)
{
	const uint8_t exp_payload_nested_mlm1[] = {MAP(1), LIST(0), END LIST(0), END END};
	const uint8_t exp_payload_nested_mlm2[] = {MAP(1), LIST(0), END LIST(1), MAP(0), END END END};
	const uint8_t exp_payload_nested_mlm3[] = {MAP(1), LIST(0), END LIST(2), MAP(0), END MAP(0), END END END};
	const uint8_t exp_payload_nested_mlm4[] = {MAP(2), LIST(0), END LIST(0), END LIST(0), END LIST(0), END END};
	const uint8_t exp_payload_nested_mlm5[] = {MAP(3), LIST(0), END LIST(0), END LIST(0), END LIST(0), END LIST(0), END LIST(2), MAP(0), END MAP(0), END END END};
	struct NestedMapListMap maplistmap1 = {
		._NestedMapListMap_key_count = 1,
		._NestedMapListMap_key = {{0}}
	};
	struct NestedMapListMap maplistmap2 = {
		._NestedMapListMap_key_count = 1,
		._NestedMapListMap_key = {
			{._NestedMapListMap_key_map_count = 1}
		}
	};
	struct NestedMapListMap maplistmap3 = {
		._NestedMapListMap_key_count = 1,
		._NestedMapListMap_key = {
			{._NestedMapListMap_key_map_count = 2}
		}
	};
	struct NestedMapListMap maplistmap4 = {
		._NestedMapListMap_key_count = 2,
		._NestedMapListMap_key = {
			{._NestedMapListMap_key_map_count = 0},
			{._NestedMapListMap_key_map_count = 0},
		}
	};
	struct NestedMapListMap maplistmap5 = {
		._NestedMapListMap_key_count = 3,
		._NestedMapListMap_key = {
			{._NestedMapListMap_key_map_count = 0},
			{._NestedMapListMap_key_map_count = 0},
			{._NestedMapListMap_key_map_count = 2},
		}
	};
	uint8_t output[30];
	size_t out_len;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_NestedMapListMap(output,
			sizeof(output), &maplistmap1, &out_len), NULL);

	zassert_equal(sizeof(exp_payload_nested_mlm1), out_len, NULL);
	zassert_mem_equal(exp_payload_nested_mlm1, output, sizeof(exp_payload_nested_mlm1), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_NestedMapListMap(output,
			sizeof(output), &maplistmap2, &out_len), NULL);

	zassert_equal(sizeof(exp_payload_nested_mlm2), out_len, "%d != %d", sizeof(exp_payload_nested_mlm2), out_len);
	zassert_mem_equal(exp_payload_nested_mlm2, output, sizeof(exp_payload_nested_mlm2), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_NestedMapListMap(output,
			sizeof(output), &maplistmap3, &out_len), NULL);

	zassert_equal(sizeof(exp_payload_nested_mlm3), out_len, "%d != %d", sizeof(exp_payload_nested_mlm3), out_len);
	zassert_mem_equal(exp_payload_nested_mlm3, output, sizeof(exp_payload_nested_mlm3), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_NestedMapListMap(output,
			sizeof(output), &maplistmap4, &out_len), NULL);

	zassert_equal(sizeof(exp_payload_nested_mlm4), out_len, NULL);
	zassert_mem_equal(exp_payload_nested_mlm4, output, sizeof(exp_payload_nested_mlm4), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_NestedMapListMap(output,
			sizeof(output), &maplistmap5, &out_len), NULL);

	zassert_equal(sizeof(exp_payload_nested_mlm5), out_len, "%d != %d", sizeof(exp_payload_nested_mlm5), out_len);
	zassert_mem_equal(exp_payload_nested_mlm5, output, sizeof(exp_payload_nested_mlm5), NULL);
}


void test_range(void)
{
	const uint8_t exp_payload_range1[] = {LIST(3),
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x0,
		END
	};

	const uint8_t exp_payload_range2[] = {LIST(6),
		0x05,
		0x08, 0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x00, 0x0A,
		END
	};

	const uint8_t exp_payload_range3[] = {LIST(5),
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x07,
		END
	};
	const uint8_t exp_payload_range4[] = {LIST(4),
		0x28,
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x0,
		END
	};

	struct Range input1 = {
		._Range_optMinus5to5_present = false,
		._Range_optStr3to6_present = false,
		._Range_optMinus9toMinus6excl_present = false,
		._Range_multi8_count = 1,
		._Range_multiHello_count = 1,
		._Range_multi0to10_count = 1,
		._Range_multi0to10 = {0},
	};
	struct Range input2 = {
		._Range_optMinus5to5_present = true,
		._Range_optMinus5to5 = 5,
		._Range_optStr3to6_present = false,
		._Range_optMinus9toMinus6excl_present = false,
		._Range_multi8_count = 2,
		._Range_multiHello_count = 1,
		._Range_multi0to10_count = 2,
		._Range_multi0to10 = {0, 10},
	};
	struct Range input3 = {
		._Range_optMinus5to5_present = false,
		._Range_optStr3to6_present = true,
		._Range_optStr3to6 = {
			.value = "hello",
			.len = 5,
		},
		._Range_optMinus9toMinus6excl_present = false,
		._Range_multi8_count = 1,
		._Range_multiHello_count = 2,
		._Range_multi0to10_count = 1,
		._Range_multi0to10 = {7},
	};
	struct Range input4 = {
		._Range_optMinus5to5_present = false,
		._Range_optStr3to6_present = false,
		._Range_optMinus9toMinus6excl_present = true,
		._Range_optMinus9toMinus6excl = -9,
		._Range_multi8_count = 1,
		._Range_multiHello_count = 1,
		._Range_multi0to10_count = 1,
		._Range_multi0to10 = {0},
	};
	struct Range input5_inv = {
		._Range_optMinus5to5_present = false,
		._Range_optStr3to6_present = false,
		._Range_optMinus9toMinus6excl_present = true,
		._Range_optMinus9toMinus6excl = -6,
		._Range_multi8_count = 1,
		._Range_multiHello_count = 1,
		._Range_multi0to10_count = 1,
		._Range_multi0to10 = {0},
	};

	uint8_t output[25];
	size_t out_len;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Range(output, sizeof(output), &input1,
				&out_len), NULL);
	zassert_equal(sizeof(exp_payload_range1), out_len, NULL);
	zassert_mem_equal(exp_payload_range1, output, sizeof(exp_payload_range1), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Range(output, sizeof(output), &input2,
				&out_len), NULL);
	zassert_equal(sizeof(exp_payload_range2), out_len, NULL);
	zassert_mem_equal(exp_payload_range2, output, sizeof(exp_payload_range2), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Range(output, sizeof(output), &input3,
				&out_len), NULL);
	zassert_equal(sizeof(exp_payload_range3), out_len, NULL);
	zassert_mem_equal(exp_payload_range3, output, sizeof(exp_payload_range3), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Range(output, sizeof(output), &input4,
				&out_len), NULL);
	zassert_equal(sizeof(exp_payload_range4), out_len, NULL);
	zassert_mem_equal(exp_payload_range4, output, sizeof(exp_payload_range4), NULL);

	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_Range(output, sizeof(output), &input5_inv,
				&out_len), NULL);
}

void test_value_range(void)
{
	const uint8_t exp_payload_value_range1[] = {LIST(6),
		11,
		0x19, 0x03, 0xe7, // 999
		0x29, // -10
		1,
		0x18, 42, // 42
		0x65, 'w', 'o', 'r', 'l', 'd', // "world"
		END
	};

	const uint8_t exp_payload_value_range2[] = {LIST(6),
		0x18, 100, // 100
		0x39, 0x03, 0xe8, // -1001
		0x18, 100, // 100
		0,
		0x18, 42, // 42
		0x65, 'w', 'o', 'r', 'l', 'd', // "world"
		END
	};

	struct ValueRange input1 = {
		._ValueRange_greater10 = 11,
		._ValueRange_less1000 = 999,
		._ValueRange_greatereqmin10 = -10,
		._ValueRange_lesseq1 = 1,
	};
	struct ValueRange input2 = {
		._ValueRange_greater10 = 100,
		._ValueRange_less1000 = -1001,
		._ValueRange_greatereqmin10 = 100,
		._ValueRange_lesseq1 = 0,
	};
	struct ValueRange input3_inval = {
		._ValueRange_greater10 = 10,
		._ValueRange_less1000 = 999,
		._ValueRange_greatereqmin10 = -10,
		._ValueRange_lesseq1 = 1,
	};
	struct ValueRange input4_inval = {
		._ValueRange_greater10 = 11,
		._ValueRange_less1000 = 1000,
		._ValueRange_greatereqmin10 = -10,
		._ValueRange_lesseq1 = 1,
	};
	struct ValueRange input5_inval = {
		._ValueRange_greater10 = 11,
		._ValueRange_less1000 = 999,
		._ValueRange_greatereqmin10 = -11,
		._ValueRange_lesseq1 = 1,
	};
	struct ValueRange input6_inval = {
		._ValueRange_greater10 = 11,
		._ValueRange_less1000 = 999,
		._ValueRange_greatereqmin10 = -10,
		._ValueRange_lesseq1 = 2,
	};
	struct ValueRange input7_inval = {
		._ValueRange_greater10 = 1,
		._ValueRange_less1000 = 999,
		._ValueRange_greatereqmin10 = -10,
		._ValueRange_lesseq1 = 1,
	};
	struct ValueRange input8_inval = {
		._ValueRange_greater10 = 11,
		._ValueRange_less1000 = 10000,
		._ValueRange_greatereqmin10 = -10,
		._ValueRange_lesseq1 = 1,
	};
	struct ValueRange input9_inval = {
		._ValueRange_greater10 = 11,
		._ValueRange_less1000 = 999,
		._ValueRange_greatereqmin10 = -100,
		._ValueRange_lesseq1 = 1,
	};
	struct ValueRange input10_inval = {
		._ValueRange_greater10 = 11,
		._ValueRange_less1000 = 999,
		._ValueRange_greatereqmin10 = -10,
		._ValueRange_lesseq1 = 21,
	};

	uint8_t output[25];
	size_t out_len;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_ValueRange(output, sizeof(output), &input1,
				&out_len), NULL);
	zassert_equal(sizeof(exp_payload_value_range1), out_len, NULL);
	zassert_mem_equal(exp_payload_value_range1, output, sizeof(exp_payload_value_range1), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_ValueRange(output, sizeof(output), &input2,
				&out_len), NULL);
	zassert_equal(sizeof(exp_payload_value_range2), out_len, NULL);
	zassert_mem_equal(exp_payload_value_range2, output, sizeof(exp_payload_value_range2), NULL);

	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_ValueRange(output, sizeof(output), &input3_inval,
				&out_len), NULL);
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_ValueRange(output, sizeof(output), &input4_inval,
				&out_len), NULL);
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_ValueRange(output, sizeof(output), &input5_inval,
				&out_len), NULL);
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_ValueRange(output, sizeof(output), &input6_inval,
				&out_len), NULL);
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_ValueRange(output, sizeof(output), &input7_inval,
				&out_len), NULL);
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_ValueRange(output, sizeof(output), &input8_inval,
				&out_len), NULL);
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_ValueRange(output, sizeof(output), &input9_inval,
				&out_len), NULL);
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_ValueRange(output, sizeof(output), &input10_inval,
				&out_len), NULL);
}

void test_single(void)
{
	uint8_t exp_payload_single0[] = {0x45, 'h', 'e', 'l', 'l', 'o'};
	uint8_t exp_payload_single1[] = {0x18, 52,};
	uint8_t exp_payload_single2[] = {9};
	uint8_t output[10];
	size_t out_len;
	struct zcbor_string input_single0 = {
		.value = "hello",
		.len = 5
	};
	uint_fast32_t input_single1 = 52;
	uint_fast32_t input_single2_ign = 53;
	uint_fast32_t input_single3 = 9;
	uint_fast32_t input_single4_inv = 10;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_SingleBstr(output, sizeof(output), &input_single0, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_single0), out_len, NULL);
	zassert_mem_equal(exp_payload_single0, output, sizeof(exp_payload_single0), NULL);
	zassert_equal(ZCBOR_ERR_NO_PAYLOAD, cbor_encode_SingleBstr(output, 5, &input_single0, &out_len), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_SingleInt(output, sizeof(output), &input_single1, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_single1), out_len, NULL);
	zassert_mem_equal(exp_payload_single1, output, sizeof(exp_payload_single1), NULL);
	zassert_equal(ZCBOR_ERR_NO_PAYLOAD, cbor_encode_SingleInt(output, 1, &input_single1, &out_len), NULL);
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_SingleInt(output, sizeof(output), &input_single2_ign, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_single1), out_len, NULL);
	zassert_mem_equal(exp_payload_single1, output, sizeof(exp_payload_single1), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_SingleInt2(output, sizeof(output), &input_single3, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_single2), out_len, NULL);
	zassert_mem_equal(exp_payload_single2, output, sizeof(exp_payload_single2), NULL);
	zassert_equal(ZCBOR_ERR_WRONG_RANGE, cbor_encode_SingleInt2(output, sizeof(output), &input_single4_inv, &out_len), NULL);
}

void test_unabstracted(void)
{
	uint8_t exp_payload_unabstracted0[] = {LIST(2), 0x01, 0x03, END};
	uint8_t exp_payload_unabstracted1[] = {LIST(2), 0x02, 0x04, END};
	struct Unabstracted result_unabstracted0 = {
		._Unabstracted_unabstractedunion1_choice = _Unabstracted_unabstractedunion1_choice1,
		._Unabstracted_unabstractedunion2_choice = _Unabstracted_unabstractedunion2_uint3,
	};
	struct Unabstracted result_unabstracted1 = {
		._Unabstracted_unabstractedunion1_choice = _Unabstracted_unabstractedunion1_choice2,
		._Unabstracted_unabstractedunion2_choice = _Unabstracted_unabstractedunion2_choice4,
	};
	uint8_t output[4];
	size_t out_len;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Unabstracted(output, sizeof(output),
					&result_unabstracted0, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_unabstracted0), out_len, "was %d\n", out_len);
	zassert_mem_equal(exp_payload_unabstracted0, output, sizeof(exp_payload_unabstracted0), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Unabstracted(output, sizeof(output),
					&result_unabstracted1, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_unabstracted1), out_len, NULL);
	zassert_mem_equal(exp_payload_unabstracted1, output, sizeof(exp_payload_unabstracted1), NULL);
}

void test_string_overflow(void)
{
	struct zcbor_string input_overflow0 = {
		.value = "",
		.len = 0xFFFFFF00, /* overflows to before this object. */
	};
	uint8_t output[10];
	size_t out_len;

	zassert_equal(ZCBOR_ERR_NO_PAYLOAD, cbor_encode_SingleBstr(output, sizeof(output), &input_overflow0, &out_len), NULL);
}

void test_quantity_range(void)
{
	uint8_t exp_payload_qty_range1[] = {0xF5, 0xF5, 0xF5};
	uint8_t exp_payload_qty_range2[] = {0xF6, 0xF6, 0xF6, 0xF6, 0xF5, 0xF5, 0xF5, 0xF5, 0xF5, 0xF5};
	struct QuantityRange result_qty_range1 = {
		._QuantityRange_upto4nils_count = 0,
		._QuantityRange_from3true_count = 3,
	};
	struct QuantityRange result_qty_range2 = {
		._QuantityRange_upto4nils_count = 4,
		._QuantityRange_from3true_count = 6,
	};
	struct QuantityRange result_qty_range3_inv = {
		._QuantityRange_upto4nils_count = 5,
		._QuantityRange_from3true_count = 3
	};
	struct QuantityRange result_qty_range4_inv = {
		._QuantityRange_upto4nils_count = 0,
		._QuantityRange_from3true_count = 2
	};
	uint8_t output[12];
	size_t out_len;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_QuantityRange(output, sizeof(output),
			&result_qty_range1, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_qty_range1), out_len, "was %d\n", out_len);
	zassert_mem_equal(exp_payload_qty_range1, output, sizeof(exp_payload_qty_range1), NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_QuantityRange(output, sizeof(output),
			&result_qty_range2, &out_len), NULL);
	zassert_equal(sizeof(exp_payload_qty_range2), out_len, "was %d\n", out_len);
	zassert_mem_equal(exp_payload_qty_range2, output, sizeof(exp_payload_qty_range2), NULL);

	zassert_equal(ZCBOR_ERR_ITERATIONS, cbor_encode_QuantityRange(output, sizeof(output),
			&result_qty_range3_inv, &out_len), NULL);

	zassert_equal(ZCBOR_ERR_ITERATIONS, cbor_encode_QuantityRange(output, sizeof(output),
			&result_qty_range4_inv, &out_len), NULL);
}

void test_doublemap(void)
{
	uint8_t exp_payload_doublemap0[] = {MAP(2), 0x01, MAP(1), 0x01, 0x01, END 0x02, MAP(1), 0x02, 0x02, END END};
	struct DoubleMap result_doublemap = {
		._DoubleMap_uintmap_count = 2,
		._DoubleMap_uintmap = {
			{
				._DoubleMap_uintmap_key = 1,
				._DoubleMap_uintmap__MyKeys = {
					._MyKeys_uint1int_present = true,
					._MyKeys_uint1int = {._MyKeys_uint1int = 1},
				}
			},
			{
				._DoubleMap_uintmap_key = 2,
				._DoubleMap_uintmap__MyKeys = {
					._MyKeys_uint2int_present = true,
					._MyKeys_uint2int = {._MyKeys_uint2int = 2},
				}
			},
		}
	};
	uint8_t output[20];
	size_t out_len;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_DoubleMap(output,
					sizeof(output),
					&result_doublemap, &out_len), NULL);
	zassert_equal(out_len, sizeof(exp_payload_doublemap0), "%d != %d\n",
			out_len, sizeof(exp_payload_doublemap0));
	zassert_mem_equal(exp_payload_doublemap0, output, out_len, NULL);
}

void test_floats(void)
{
	uint8_t exp_floats_payload2[] = {LIST(4), 0xFA, 0xc7, 0xc0, 0xe6, 0xb7 /* -98765.4321 */,
			0xFB, 0x41, 0x32, 0xd6, 0x87, 0xe3, 0xd7, 0xa, 0x3d /* 1234567.89 */,
			0xFB, 0x40, 0x9, 0x21, 0xca, 0xc0, 0x83, 0x12, 0x6f /* 3.1415 */,
			0xFB, 0x40, 0x5, 0xbf, 0x9, 0x95, 0xaa, 0xf7, 0x90 /* 2.71828 */,
			END
	};

	uint8_t exp_floats_payload3[] = {LIST(4), 0xFA, 0x49, 0x96, 0xb4, 0x3f /* 1234567.89 */,
			0xFB, 0xc0, 0xf8, 0x1c, 0xd6, 0xe9, 0xe1, 0xb0, 0x8a /* -98765.4321 */,
			0xFB, 0x40, 0x9, 0x21, 0xca, 0xc0, 0x83, 0x12, 0x6f /* 3.1415 */,
			0xFB, 0x40, 0x5, 0xbf, 0x9, 0x95, 0xaa, 0xf7, 0x90 /* 2.71828 */,
			END
	};

	uint8_t exp_floats_payload4[] = {LIST(7), 0xFA, 0x49, 0x96, 0xb4, 0x3f /* 1234567.89 */,
			0xFB, 0xc0, 0xf8, 0x1c, 0xd6, 0xe9, 0xe1, 0xb0, 0x8a /* -98765.4321 */,
			0xFB, 0x40, 0x9, 0x21, 0xca, 0xc0, 0x83, 0x12, 0x6f /* 3.1415 */,
			0xFB, 0x40, 0x5, 0xbf, 0x9, 0x95, 0xaa, 0xf7, 0x90 /* 2.71828 */,
			0xFB, 0x3f, 0x31, 0xa5, 0x9d, 0xe0, 0x0, 0x0, 0x0 /* 123/456789 */,
			0xFB, 0x3f, 0x31, 0xa5, 0x9d, 0xd9, 0x57, 0x14, 0x64 /* 123/456789 */,
			0xFB, 0xbd, 0x50, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 /* -2^(-42) */,
			END
	};

	struct Floats input;
	size_t num_encode;
	uint8_t output[70];

	input._Floats_float_32 = (float)-98765.4321;
	input._Floats_float_64 = (double)1234567.89;
	input._Floats_floats_count = 0;
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Floats(
		output, sizeof(output), &input, &num_encode), NULL);
	zassert_equal(sizeof(exp_floats_payload2), num_encode, NULL);
	zassert_mem_equal(exp_floats_payload2, output, num_encode, NULL);

	input._Floats_float_32 = (float)1234567.89;
	input._Floats_float_64 = (double)-98765.4321;
	input._Floats_floats_count = 0;
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Floats(
		output, sizeof(output), &input, &num_encode), NULL);
	zassert_equal(sizeof(exp_floats_payload3), num_encode, NULL);
	zassert_mem_equal(exp_floats_payload3, output, num_encode, NULL);

	input._Floats_float_32 = (float)1234567.89;
	input._Floats_float_64 = (double)-98765.4321;
	input._Floats_floats_count = 3;
	input._Floats_floats[0] = (float)(123.0/456789.0);
	input._Floats_floats[1] = (double)(123.0/456789.0);
	input._Floats_floats[2] = (double)(-1.0/(1LL << 42));
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Floats(
		output, sizeof(output), &input, &num_encode), NULL);
	zassert_equal(sizeof(exp_floats_payload4), num_encode, NULL);
	zassert_mem_equal(exp_floats_payload4, output, num_encode, NULL);

}

void test_cbor_bstr(void)
{
	uint8_t exp_cbor_bstr_payload1[] = {
#ifdef ZCBOR_CANONICAL
		0x58, 31,
#else
		0x58, 32,
#endif
			LIST(3),
				0x46, 0x65, 'H', 'e', 'l', 'l', 'o',
				0x49, 0xFB, 0x40, 0x9, 0x21, 0xca, 0xc0, 0x83, 0x12, 0x6f /* 3.1415 */,
				0x4C, 0xC2, 0x4A, 0x42, 2, 3, 4, 5, 6, 7, 8, 9, 10 /* 0x4202030405060708090A */,
			END
	};

	struct CBORBstr input = {0};
	size_t num_encode;
	uint8_t output[70];

	input.__hello_big_uint_bstr_cbor.value = (uint8_t []){0x42, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	input.__hello_big_uint_bstr_cbor.len = 10;

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_CBORBstr(output, sizeof(output), &input, &num_encode), NULL);

	zcbor_print_compare_strings(exp_cbor_bstr_payload1, output, num_encode);

	zassert_equal(sizeof(exp_cbor_bstr_payload1), num_encode, "%d != %d\r\n", sizeof(exp_cbor_bstr_payload1), num_encode);
	zassert_mem_equal(exp_cbor_bstr_payload1, output, num_encode, NULL);
}

void test_main(void)
{
	ztest_test_suite(cbor_encode_test3,
			 ztest_unit_test(test_numbers),
			 ztest_unit_test(test_numbers2),
			 ztest_unit_test(test_number_map),
			 ztest_unit_test(test_strings),
			 ztest_unit_test(test_primitives),
			 ztest_unit_test(test_optional),
			 ztest_unit_test(test_union),
			 ztest_unit_test(test_levels),
			 ztest_unit_test(test_map),
			 ztest_unit_test(test_nested_list_map),
			 ztest_unit_test(test_nested_map_list_map),
			 ztest_unit_test(test_range),
			 ztest_unit_test(test_value_range),
			 ztest_unit_test(test_single),
			 ztest_unit_test(test_unabstracted),
			 ztest_unit_test(test_string_overflow),
			 ztest_unit_test(test_quantity_range),
			 ztest_unit_test(test_doublemap),
			 ztest_unit_test(test_floats),
			 ztest_unit_test(test_cbor_bstr)
	);
	ztest_run_test_suite(cbor_encode_test3);
}
