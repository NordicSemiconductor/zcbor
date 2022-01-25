/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include "strange.h"
#include "zcbor_debug.h" // Enables use of print functions when debugging tests.

#define CONCAT_BYTE(a,b) a ## b

#ifdef TEST_INDETERMINATE_LENGTH_ARRAYS
#define LIST(num) 0x9F /* Use short count 31 (indefinite-length). Note that the 'num' argument is ignored */
#define MAP(num) 0xBF  /* Use short count 31 (indefinite-length). Note that the 'num' argument is ignored */
#define END 0xFF,
#define STR_LEN(len, lists) (len + lists)
#else
#define LIST(num) CONCAT_BYTE(0x8, num)
#define MAP(num) CONCAT_BYTE(0xA, num)
#define END
#define STR_LEN(len, lists) (len)
#endif


void test_numbers(void)
{
	size_t decode_len = 0xFFFFFFFF;
	const uint8_t payload_numbers1[] = {
		LIST(A),
			0x01, // 1
			0x21, // -2
			0x05, // 5
			0x19, 0x01, 0x00, // 256
			0x1A, 0x01, 0x02, 0x03, 0x04, // 0x01020304
			0x39, 0x13, 0x87, // -5000
			0x1A, 0xEE, 0x6B, 0x28, 0x00, // 4000000000
			0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -2^31
			0x00, // 0
			0xD9, 0xFF, 0xFF, 0x01, // 1 tagged (0xFFFF)
		END
	};

	struct Numbers numbers;
	zassert_false(cbor_decode_Numbers(payload_numbers1,
		sizeof(payload_numbers1) - 1, &numbers, &decode_len), NULL); // Payload too small.
	zassert_equal(0xFFFFFFFF, decode_len, NULL); // Should be untouched
	zassert_true(cbor_decode_Numbers(payload_numbers1,
		sizeof(payload_numbers1) + 1, &numbers, &decode_len), NULL); // Payload too large
	zassert_equal(sizeof(payload_numbers1), decode_len, NULL);

	zassert_equal(5, numbers._Numbers_fourtoten, NULL);
	zassert_equal(256, numbers._Numbers_twobytes, NULL);
	zassert_equal(0x01020304, numbers._Numbers_onetofourbytes, NULL);
	zassert_equal(-5000, numbers._Numbers_minusfivektoplustwohundred, "%d", numbers._Numbers_minusfivektoplustwohundred);
	zassert_equal(-2147483648, numbers._Numbers_negint, NULL);
	zassert_equal(0, numbers._Numbers_posint, NULL);
	zassert_equal(1, numbers._Numbers_integer, NULL);
}


void test_numbers2(void)
{
	size_t decode_len = 0xFFFFFFFF;
	const uint8_t payload_numbers2[] = {
		LIST(5),
			0x1A, 0x00, 0x12, 0x34, 0x56, // 0x123456
			0x1B, 0x01, 2, 3, 4, 5, 6, 7, 8, // 0x0102030405060708
			0x1B, 0x11, 2, 3, 4, 5, 6, 7, 9, // 0x1102030405060709
			0x3A, 0x80, 0x00, 0x00, 0x00, // -0x8000_0001
			0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -0x8000_0000
		END
	};
	const uint8_t payload_numbers2_1[] = {
		LIST(5),
			0x1A, 0x00, 0x12, 0x34, 0x56, // 0x123456
			0x3B, 0x01, 2, 3, 4, 5, 6, 7, 8, // -0x0102030405060709
			0x1B, 0x11, 2, 3, 4, 5, 6, 7, 9, // 0x1102030405060709
			0x3A, 0x80, 0x00, 0x00, 0x00, // -0x8000_0001
			0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -0x8000_0000
		END
	};
	const uint8_t payload_numbers2_inv2[] = {
		LIST(5),
			0x1A, 0x00, 0x12, 0x34, 0x56, // 0x123456
			0x1B, 0x01, 2, 3, 4, 5, 6, 7, 8, // 0x0102030405060708
			0x1B, 0x11, 2, 3, 4, 5, 6, 7, 9, // 0x1102030405060709
			0x3A, 0x80, 0x00, 0x00, 0x01, // -0x8000_0002 INV
			0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -0x8000_0000
		END
	};
	const uint8_t payload_numbers2_inv3[] = {
		LIST(5),
			0x1A, 0x00, 0x12, 0x34, 0x56, // 0x123456
			0x1B, 0x01, 2, 3, 4, 5, 6, 7, 8, // 0x0102030405060708
			0x1B, 0x11, 2, 3, 4, 5, 6, 7, 9, // 0x1102030405060709
			0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -0x8000_0000 INV
			0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -0x8000_0000
		END
	};
	const uint8_t payload_numbers2_inv4[] = {
		LIST(5),
			0x1A, 0x00, 0x12, 0x34, 0x56, // 0x123456
			0x1B, 0x01, 2, 3, 4, 5, 6, 7, 8, // 0x0102030405060708
			0x1A, 0x00, 0x12, 0x34, 0x56, // 0x123456 INV
			0x3A, 0x80, 0x00, 0x00, 0x00, // -0x8000_0001
			0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -0x8000_0000
		END
	};
	const uint8_t payload_numbers2_inv5[] = {
		LIST(5),
			0x1A, 0x00, 0x12, 0x34, 0x56, // 0x123456
			0x1B, 0x01, 2, 3, 4, 5, 6, 7, 8, // 0x0102030405060708
			0x1B, 0x11, 2, 3, 4, 5, 6, 7, 9, // 0x1102030405060709
			0x3A, 0x80, 0x00, 0x00, 0x00, // -0x8000_0001
			0x3A, 0x80, 0x00, 0x00, 0x00, // -0x8000_0001 INV
		END
	};
	struct Numbers2 numbers2;

	zassert_true(cbor_decode_Numbers2(payload_numbers2,
		sizeof(payload_numbers2), &numbers2, &decode_len), NULL);

	zassert_equal(0x123456, numbers2._Numbers2_threebytes, NULL);
	zassert_equal(0x0102030405060708, numbers2._Numbers2_bigint, NULL);
	zassert_equal(0x1102030405060709, numbers2._Numbers2_biguint, NULL);

	zassert_true(cbor_decode_Numbers2(payload_numbers2_1,
		sizeof(payload_numbers2_1), &numbers2, &decode_len), NULL);

	zassert_equal(0x123456, numbers2._Numbers2_threebytes, NULL);
	zassert_equal(-0x0102030405060709, numbers2._Numbers2_bigint, NULL);
	zassert_equal(0x1102030405060709, numbers2._Numbers2_biguint, NULL);

	zassert_false(cbor_decode_Numbers2(payload_numbers2_inv2,
		sizeof(payload_numbers2_inv2), &numbers2, &decode_len), NULL);

	zassert_false(cbor_decode_Numbers2(payload_numbers2_inv3,
		sizeof(payload_numbers2_inv3), &numbers2, &decode_len), NULL);

	zassert_false(cbor_decode_Numbers2(payload_numbers2_inv4,
		sizeof(payload_numbers2_inv4), &numbers2, &decode_len), NULL);

	zassert_false(cbor_decode_Numbers2(payload_numbers2_inv5,
		sizeof(payload_numbers2_inv5), &numbers2, &decode_len), NULL);
}

void test_strings(void)
{
	const uint8_t payload_strings1[] = {
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
		0x58, STR_LEN(29, 1), // Numbers (len: 29)
			LIST(A),
				0x01, // 1
				0x21, // -2
				0x05, // 5
				0x19, 0xFF, 0xFF, // 0xFFFF
				0x18, 0x18, // 24
				0x00, // 0
				0x1A, 0xEE, 0x6B, 0x28, 0x00, // 4000000000
				0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -2^31
				0x1A, 0xFF, 0xFF, 0xFF, 0xFF, // 0xFFFFFFFF
				0xD9, 0xFF, 0xFF, 0x09, // 9, tagged (0xFFFF)
			END
		STR_LEN(0x52, 3), // Primitives (len: 18)
			LIST(5),
				0xF5, // True
				0xF4, // False
				0xF4, // False
				0xF6, // Nil
				0xF7, // Undefined
			END
			LIST(5),
				0xF5, // True
				0xF4, // False
				0xF5, // True
				0xF6, // Nil
				0xF7, // Undefined
			END
			LIST(5),
				0xF5, // True
				0xF4, // False
				0xF4, // False
				0xF6, // Nil
				0xF7, // Undefined
			END
		0x59, 0x01, STR_LEN(0x68, 3), // Strings (len: 360)
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
			0x58, STR_LEN(29, 1), // Numbers (len: 29)
				LIST(A),
					0x01, // 1
					0x21, // -2
					0x05, // 5
					0x19, 0xFF, 0xFF, // 0xFFFF
					0x18, 0x18, // 24
					0x00, // 0
					0x1A, 0xEE, 0x6B, 0x28, 0x00, // 4000000000
					0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -2^31
					0x1A, 0xFF, 0xFF, 0xFF, 0xFF, // 0xFFFFFFFF
					0xD9, 0xFF, 0xFF, 0x29, // -10, tagged (0xFFFF)
				END
			STR_LEN(0x46, 1), // Primitives (len: 6)
				LIST(5),
					0xF5, // True
					0xF4, // False
					0xF4, // False
					0xF6, // Nil
					0xF7, // Undefined
				END
			END
		END
	};

	struct Strings strings1;
	struct Strings strings2;
	struct Numbers numbers1;
	struct Numbers numbers2;
	zassert_true(cbor_decode_Strings(payload_strings1,
		sizeof(payload_strings1), &strings1, NULL), NULL);
	zassert_true(strings1._Strings_optCborStrings_present, NULL);
	zassert_true(cbor_decode_Strings(strings1._Strings_optCborStrings.value,
		strings1._Strings_optCborStrings.len, &strings2, NULL), NULL);
	zassert_true(cbor_decode_Numbers(strings1._Strings_cborNumbers.value,
		strings1._Strings_cborNumbers.len, &numbers1, NULL), NULL);
	zassert_true(cbor_decode_Numbers(strings2._Strings_cborNumbers.value,
		strings2._Strings_cborNumbers.len, &numbers2, NULL), NULL);

	zassert_equal(300, strings1._Strings_threehundrebytebstr.len, NULL);
	zassert_equal(0, strings1._Strings_threehundrebytebstr.value[0], NULL);
	zassert_equal(9, strings1._Strings_threehundrebytebstr.value[299], NULL);
	zassert_equal(30, strings1._Strings_tentothirtybytetstr.len, NULL);
	zassert_equal(STR_LEN(29, 1), strings1._Strings_cborNumbers.len, NULL);
	zassert_equal(3, strings1._Strings_cborseqPrimitives_cbor_count, NULL);
	zassert_false(strings1._Strings_cborseqPrimitives_cbor[0]._Primitives_boolval, NULL);
	zassert_true(strings1._Strings_cborseqPrimitives_cbor[1]._Primitives_boolval, NULL);
	zassert_false(strings1._Strings_cborseqPrimitives_cbor[2]._Primitives_boolval, NULL);

	zassert_equal(300, strings2._Strings_threehundrebytebstr.len, NULL);
	zassert_equal(10, strings2._Strings_tentothirtybytetstr.len, NULL);
	zassert_equal(5, numbers2._Numbers_fourtoten, NULL);
	zassert_equal(0xFFFF, numbers2._Numbers_twobytes, NULL);
	zassert_equal(24, numbers2._Numbers_onetofourbytes, NULL);
	zassert_equal(0, numbers2._Numbers_minusfivektoplustwohundred, NULL);
	zassert_equal(-2147483648, numbers2._Numbers_negint, NULL);
	zassert_equal(0xFFFFFFFF, numbers2._Numbers_posint, NULL);
	zassert_equal(-10, numbers2._Numbers_integer, NULL);
	zassert_equal(1, strings2._Strings_cborseqPrimitives_cbor_count, NULL);
	zassert_false(strings2._Strings_cborseqPrimitives_cbor[0]._Primitives_boolval, NULL);
}

void test_primitives(void)
{
	uint8_t payload_prim1[] = {LIST(5), 0xF5, 0xF4, 0xF4, 0xF6, 0xF7, END};
	uint8_t payload_prim2[] = {LIST(5), 0xF5, 0xF4, 0xF5, 0xF6, 0xF7, END};
	uint8_t payload_prim_inv3[] = {LIST(5), 0xF7, 0xF4, 0xF4, 0xF6, 0xF7, END};
	uint8_t payload_prim_inv4[] = {LIST(5), 0xF5, 0xF7, 0xF4, 0xF6, 0xF7, END};
	uint8_t payload_prim_inv5[] = {LIST(5), 0xF5, 0xF4, 0xF7, 0xF6, 0xF7, END};
	uint8_t payload_prim_inv6[] = {LIST(5), 0xF5, 0xF4, 0xF5, 0xF7, 0xF7, END};
	uint8_t payload_prim_inv7[] = {LIST(5), 0xF5, 0xF4, 0xF5, 0xF6, 0xF6, END};
	uint8_t payload_prim_inv8[] = {LIST(5), 0xF5, 0xF4, 0xF5, 0xF6, 0xF5, END};
	uint8_t payload_prim_inv9[] = {LIST(5), 0xF5, 0xF4, 0xF6, 0xF6, 0xF7, END};
	uint8_t payload_prim_inv10[] = {LIST(5), 0xF5, 0xF5, 0xF5, 0xF6, 0xF7, END};
	uint8_t payload_prim_inv11[] = {LIST(5), 0xF4, 0xF4, 0xF5, 0xF6, 0xF7, END};

	struct Primitives result;
	size_t len_decode;

	zassert_true(cbor_decode_Prim2(payload_prim1, sizeof(payload_prim1), &result, &len_decode), NULL);
	zassert_true(cbor_decode_Prim2(payload_prim2, sizeof(payload_prim2), &result, &len_decode), NULL);
	zassert_false(cbor_decode_Prim2(payload_prim_inv3, sizeof(payload_prim_inv3), &result, &len_decode), NULL);
	zassert_false(cbor_decode_Prim2(payload_prim_inv4, sizeof(payload_prim_inv4), &result, &len_decode), NULL);
	zassert_false(cbor_decode_Prim2(payload_prim_inv5, sizeof(payload_prim_inv5), &result, &len_decode), NULL);
	zassert_false(cbor_decode_Prim2(payload_prim_inv6, sizeof(payload_prim_inv6), &result, &len_decode), NULL);
	zassert_false(cbor_decode_Prim2(payload_prim_inv7, sizeof(payload_prim_inv7), &result, &len_decode), NULL);
	zassert_false(cbor_decode_Prim2(payload_prim_inv8, sizeof(payload_prim_inv8), &result, &len_decode), NULL);
	zassert_false(cbor_decode_Prim2(payload_prim_inv9, sizeof(payload_prim_inv9), &result, &len_decode), NULL);
	zassert_false(cbor_decode_Prim2(payload_prim_inv10, sizeof(payload_prim_inv10), &result, &len_decode), NULL);
	zassert_false(cbor_decode_Prim2(payload_prim_inv11, sizeof(payload_prim_inv11), &result, &len_decode), NULL);
}

void test_string_overflow(void)
{
	const uint8_t payload_overflow[] = {
		0x5A, 0xFF, 0xFF, 0xF0, 0x00, /* overflows to before this string. */
	};

	struct zcbor_string result_overflow;
	size_t out_len;

	zassert_false(cbor_decode_SingleBstr(payload_overflow, sizeof(payload_overflow), &result_overflow, &out_len), NULL);
}

void test_optional(void)
{
	const uint8_t payload_optional1[] = {
		LIST(3), 0xCA /* tag */, 0xF4 /* False */, 0x02, 0x03,
		END
	};
	const uint8_t payload_optional2[] = {
		LIST(2), 0xCA /* tag */, 0xF4 /* False */, 0x03,
		END
	};
	const uint8_t payload_optional3_inv[] = {
		LIST(2), 0xCA /* tag */, 0xF4 /* False */, 0x02,
		END
	};
	const uint8_t payload_optional4[] = {
		LIST(3), 0xCA /* tag */, 0xF4 /* False */, 0x02, 0x01,
		END
	};
	const uint8_t payload_optional5[] = {
		LIST(3), 0xCA /* tag */, 0xF5 /* True */, 0x02, 0x02,
		END
	};
	const uint8_t payload_optional6[] = {
		LIST(4), 0xCA /* tag */, 0xF5 /* True */, 0xF4 /* False */, 0x02, 0x02,
		END
	};
	const uint8_t payload_optional7_inv[] = {
		LIST(2), 0xCB /* wrong tag */, 0xF4 /* False */, 0x03,
		END
	};
	const uint8_t payload_optional8_inv[] = {
		LIST(2), 0xF4 /* False (no tag first) */, 0x03,
		END
	};
	const uint8_t payload_optional9[] = {
		LIST(4), 0xCA /* tag */, 0xF4 /* False */, 0x02, 0x03, 0x08,
		END
	};
	const uint8_t payload_optional10[] = {
		LIST(6), 0xCA /* tag */, 0xF4 /* False */, 0x02, 0x03, 0x08, 0x08, 0x08,
		END
	};
	const uint8_t payload_optional11_inv[] = {
		LIST(6), 0xCA /* tag */, 0xF4 /* False */, 0x02, 0x03, 0x08, 0x08, 0x09,
		END
	};

	struct Optional optional;
	zassert_true(cbor_decode_Optional(payload_optional1,
			sizeof(payload_optional1), &optional, NULL), NULL);
	zassert_false(optional._Optional_boolval, NULL);
	zassert_false(optional._Optional_optbool_present, NULL);
	zassert_true(optional._Optional_opttwo_present, NULL);
	zassert_equal(3, optional._Optional_manduint, NULL);
	zassert_equal(0, optional._Optional_multi8_count, NULL);

	zassert_true(cbor_decode_Optional(payload_optional2,
			sizeof(payload_optional2), &optional, NULL), NULL);
	zassert_false(optional._Optional_boolval, NULL);
	zassert_false(optional._Optional_optbool_present, NULL);
	zassert_false(optional._Optional_opttwo_present, NULL);
	zassert_equal(3, optional._Optional_manduint, NULL);
	zassert_equal(0, optional._Optional_multi8_count, NULL);

	zassert_false(cbor_decode_Optional(payload_optional3_inv,
			sizeof(payload_optional3_inv), &optional, NULL), NULL);

	zassert_true(cbor_decode_Optional(payload_optional4,
			sizeof(payload_optional4), &optional, NULL), NULL);
	zassert_false(optional._Optional_boolval, NULL);
	zassert_false(optional._Optional_optbool_present, NULL);
	zassert_true(optional._Optional_opttwo_present, NULL);
	zassert_equal(1, optional._Optional_manduint, NULL);
	zassert_equal(0, optional._Optional_multi8_count, NULL);

	zassert_true(cbor_decode_Optional(payload_optional5,
			sizeof(payload_optional5), &optional, NULL), NULL);
	zassert_true(optional._Optional_boolval, NULL);
	zassert_false(optional._Optional_optbool_present, NULL);
	zassert_true(optional._Optional_opttwo_present, NULL);
	zassert_equal(2, optional._Optional_manduint, NULL);
	zassert_equal(0, optional._Optional_multi8_count, NULL);

	zassert_true(cbor_decode_Optional(payload_optional6,
			sizeof(payload_optional6), &optional, NULL), NULL);
	zassert_true(optional._Optional_boolval, NULL);
	zassert_true(optional._Optional_optbool_present, NULL);
	zassert_false(optional._Optional_optbool, NULL);
	zassert_true(optional._Optional_opttwo_present, NULL);
	zassert_equal(2, optional._Optional_manduint, NULL);
	zassert_equal(0, optional._Optional_multi8_count, NULL);

	zassert_false(cbor_decode_Optional(payload_optional7_inv,
			sizeof(payload_optional7_inv), &optional, NULL), NULL);

	zassert_false(cbor_decode_Optional(payload_optional8_inv,
			sizeof(payload_optional8_inv), &optional, NULL), NULL);

	zassert_true(cbor_decode_Optional(payload_optional9,
			sizeof(payload_optional9), &optional, NULL), NULL);
	zassert_false(optional._Optional_boolval, NULL);
	zassert_false(optional._Optional_optbool_present, NULL);
	zassert_true(optional._Optional_opttwo_present, NULL);
	zassert_equal(3, optional._Optional_manduint, NULL);
	zassert_equal(1, optional._Optional_multi8_count, NULL);

	zassert_true(cbor_decode_Optional(payload_optional10,
			sizeof(payload_optional10), &optional, NULL), NULL);
	zassert_false(optional._Optional_boolval, NULL);
	zassert_false(optional._Optional_optbool_present, NULL);
	zassert_true(optional._Optional_opttwo_present, NULL);
	zassert_equal(3, optional._Optional_manduint, NULL);
	zassert_equal(3, optional._Optional_multi8_count, NULL);

	zassert_false(cbor_decode_Optional(payload_optional11_inv,
			sizeof(payload_optional11_inv), &optional, NULL), NULL);
}

void test_union(void)
{
	const uint8_t payload_union1[] = {0x01, 0x21};
	const uint8_t payload_union2[] = {0x03, 0x23};
	const uint8_t payload_union3[] = {0x03, 0x04};
	const uint8_t payload_union4[] = {
		0x67, 0x22, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x22 // "hello"
	};
	const uint8_t payload_union6[] = {
		0x03, 0x23, 0x03, 0x23, 0x03, 0x23,
		0x03, 0x23, 0x03, 0x23, 0x03, 0x23
	};
	const uint8_t payload_union7_long[] = {0x03, 0x23, 0x03, 0x04};
	const uint8_t payload_union8_long[] = {0x03, 0x23, 0x03};
	const uint8_t payload_union9_long[] = {0x03, 0x04, 0x03, 0x04};
	const uint8_t payload_union10_inv[] = {
		0x03, 0x23, 0x03, 0x23, 0x03, 0x23, 0x03, 0x23,
		0x03, 0x23, 0x03, 0x23, 0x03, 0x23}; /* Too many */
	size_t decode_len;

	struct Union_ _union;
	zassert_true(cbor_decode_Union(payload_union1, sizeof(payload_union1),
				&_union, NULL), NULL);
	zassert_equal(_Union__Group, _union._Union_choice, NULL);

	zassert_true(cbor_decode_Union(payload_union2, sizeof(payload_union2),
				&_union, NULL), NULL);
	zassert_equal(_Union__MultiGroup, _union._Union_choice, NULL);
	zassert_equal(1, _union._Union__MultiGroup._MultiGroup_count, "was %d\r\n", _union._Union__MultiGroup._MultiGroup_count);

	zassert_true(cbor_decode_Union(payload_union3, sizeof(payload_union3),
				&_union, NULL), NULL);
	zassert_equal(_Union__uint3, _union._Union_choice, NULL);

	zassert_true(cbor_decode_Union(payload_union4, sizeof(payload_union4),
				&_union, NULL), NULL);
	zassert_equal(_Union_hello_tstr, _union._Union_choice, NULL);

	zassert_true(cbor_decode_Union(payload_union6, sizeof(payload_union6),
				&_union, &decode_len), NULL);
	zassert_equal(_Union__MultiGroup, _union._Union_choice, NULL);
	zassert_equal(6, _union._Union__MultiGroup._MultiGroup_count, NULL);
	zassert_equal(12, decode_len, NULL);

	zassert_true(cbor_decode_Union(payload_union7_long, sizeof(payload_union7_long),
				&_union, &decode_len), NULL);
	zassert_equal(2, decode_len, NULL);
	zassert_true(cbor_decode_Union(payload_union8_long, sizeof(payload_union8_long),
				&_union, &decode_len), NULL);
	zassert_equal(2, decode_len, NULL);
	zassert_true(cbor_decode_Union(payload_union9_long, sizeof(payload_union9_long),
				&_union, &decode_len), NULL);
	zassert_equal(2, decode_len, NULL);

	zassert_true(cbor_decode_Union(payload_union10_inv, sizeof(payload_union10_inv),
				&_union, &decode_len), NULL);
	zassert_equal(6, _union._Union__MultiGroup._MultiGroup_count, NULL);
	zassert_equal(12, decode_len, NULL);
}

void test_levels(void)
{
	const uint8_t payload_levels1[] = {
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

	struct Level2 level1;
	zassert_true(cbor_decode_Level1(payload_levels1,
		sizeof(payload_levels1), &level1, NULL), NULL);

	zassert_equal(2, level1._Level2__Level3_count, NULL);
	zassert_equal(4, level1._Level2__Level3[0]._Level3__Level4_count, NULL);
	zassert_equal(4, level1._Level2__Level3[1]._Level3__Level4_count, NULL);
}


void test_map(void)
{
	const uint8_t payload_map1[] = {
		MAP(4), LIST(2), 0x05, 0x06, END 0xF4, // [5,6] => false
		0x07, 0x01, // 7 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
		0xf6, 0x40, // nil => ""
		END
	};
	const uint8_t payload_map2_inv[] = {
		MAP(4), LIST(2), 0x05, 0x06, END 0xF4, // [5,6] => false
		0x07, 0x01, // 7 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
		END
	};
	const uint8_t payload_map3[] = {
		MAP(5), LIST(2), 0x05, 0x06, END 0xF5, // [5,6] => true
		0x07, 0x01, // 7 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
		0xf6, 0x40, // nil => ""
		0xf6, 0x40, // nil => ""
		END
	};

	/* Wrong list length. Will not fail for indeterminate length arrays. */
	const uint8_t payload_map4_inv[] = {
		MAP(6), LIST(2), 0x05, 0x06, END 0xF4, // [5,6] => false
		0x07, 0x01, // 7 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
		0xf6, 0x40, // nil => ""
		0xf6, 0x40, // nil => ""
		END
	};
	const uint8_t payload_map5[] = {
		MAP(4), LIST(2), 0x05, 0x06, END 0xF4, // [5,6] => false
		0x27, 0x01, // -8 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
		0xf6, 0x40, // nil => ""
		END
	};

	struct Map map;

	zassert_true(cbor_decode_Map(payload_map1, sizeof(payload_map1),
			&map, NULL), NULL);
	zassert_false(map._Map_key, NULL);
	zassert_equal(_union_uint7uint, map._Map_union_choice, NULL);
	zassert_equal(1, map._union_uint7uint, NULL);
	zassert_equal(2, map._Map_twotothree_count, NULL);
	zassert_equal(5, map._Map_twotothree[0]._Map_twotothree.len, NULL);
	zassert_mem_equal("hello", map._Map_twotothree[0]._Map_twotothree.value,
			5, NULL);
	zassert_equal(0, map._Map_twotothree[1]._Map_twotothree.len, NULL);

	zassert_false(cbor_decode_Map(payload_map2_inv, sizeof(payload_map2_inv),
			&map, NULL), NULL);

	zassert_true(cbor_decode_Map(payload_map3, sizeof(payload_map3),
			&map, NULL), NULL);
	zassert_true(map._Map_key, NULL);
	zassert_equal(_union_uint7uint, map._Map_union_choice, NULL);
	zassert_equal(1, map._union_uint7uint, NULL);
	zassert_equal(3, map._Map_twotothree_count, NULL);
	zassert_equal(5, map._Map_twotothree[0]._Map_twotothree.len, NULL);
	zassert_mem_equal("hello", map._Map_twotothree[0]._Map_twotothree.value,
			5, NULL);
	zassert_equal(0, map._Map_twotothree[1]._Map_twotothree.len, NULL);
	zassert_equal(0, map._Map_twotothree[2]._Map_twotothree.len, NULL);

#ifdef TEST_INDETERMINATE_LENGTH_ARRAYS
	zassert_true(
#else
	zassert_false(
#endif
		cbor_decode_Map(payload_map4_inv, sizeof(payload_map4_inv),
			&map, NULL), NULL);

	zassert_true(cbor_decode_Map(payload_map5, sizeof(payload_map5),
			&map, NULL), NULL);
	zassert_false(map._Map_key, NULL);
	zassert_equal(_union_nintuint, map._Map_union_choice, NULL);
	zassert_equal(1, map._union_nintuint, NULL);
	zassert_equal(2, map._Map_twotothree_count, NULL);
}


static bool my_decode_EmptyMap(zcbor_state_t *state, void *unused)
{
	size_t payload_len_out;
	bool res = cbor_decode_EmptyMap(state->payload,
		state->payload_end - state->payload, NULL, &payload_len_out);

	if (res) {
		state->payload += payload_len_out;
	}
	return res;
}


void test_empty_map(void)
{
	const uint8_t payload1[] = {MAP(0), END};
	const uint8_t payload2_inv[] = {MAP(1), END};
	const uint8_t payload3_inv[] = {MAP(1), 0, END};
	const uint8_t payload4[] = {MAP(0), END MAP(0), END MAP(0), END};
	uint_fast32_t num_decode;
	zcbor_state_t state;

	zcbor_new_state(&state, 1, payload4, sizeof(payload4), 3);

	zassert_true(cbor_decode_EmptyMap(payload1, sizeof(payload1), NULL, NULL), NULL);
#ifdef TEST_INDETERMINATE_LENGTH_ARRAYS
	zassert_true(cbor_decode_EmptyMap(payload2_inv, sizeof(payload2_inv), NULL, NULL), NULL);
#else
	zassert_false(cbor_decode_EmptyMap(payload2_inv, sizeof(payload2_inv), NULL, NULL), NULL);
#endif
	zassert_false(cbor_decode_EmptyMap(payload3_inv, sizeof(payload3_inv), NULL, NULL), NULL);
	zassert_true(zcbor_multi_decode(3, 3, &num_decode, my_decode_EmptyMap, &state, NULL, 0), NULL);
	zassert_equal(3, num_decode, NULL);
}


void test_nested_list_map(void)
{
	const uint8_t payload_nested_lm1[] = {LIST(0), END};
	const uint8_t payload_nested_lm2[] = {LIST(1), MAP(0), END END};
	const uint8_t payload_nested_lm3[] = {LIST(1), MAP(1), 0x01, 0x04, END END};
	const uint8_t payload_nested_lm4_inv[] = {LIST(1), MAP(2), 0x01, 0x04, 0x1, 0x4, END END};
	const uint8_t payload_nested_lm5[] = {LIST(2), MAP(0), END MAP(1), 0x01, 0x04, END END};
	const uint8_t payload_nested_lm6_inv[] = {LIST(2), MAP(0), END MAP(1), 0x04, END END};
	const uint8_t payload_nested_lm7[] = {LIST(3), MAP(0), END MAP(0), END MAP(0), END END};
	struct NestedListMap listmap;

	zassert_true(cbor_decode_NestedListMap(payload_nested_lm1,
			sizeof(payload_nested_lm1), &listmap, NULL), NULL);
	zassert_equal(0, listmap._NestedListMap_map_count, NULL);
	zassert_true(cbor_decode_NestedListMap(payload_nested_lm2,
			sizeof(payload_nested_lm2), &listmap, NULL), NULL);
	zassert_equal(1, listmap._NestedListMap_map_count, NULL);
	zassert_false(listmap._NestedListMap_map[0]._map_uint4_present, NULL);
	zassert_true(cbor_decode_NestedListMap(payload_nested_lm3,
			sizeof(payload_nested_lm3), &listmap, NULL), NULL);
	zassert_equal(1, listmap._NestedListMap_map_count, NULL);
	zassert_true(listmap._NestedListMap_map[0]._map_uint4_present, NULL);
	zassert_equal(1, listmap._NestedListMap_map[0]._map_uint4_present, NULL);
	zassert_false(cbor_decode_NestedListMap(payload_nested_lm4_inv,
			sizeof(payload_nested_lm4_inv), &listmap, NULL), NULL);
	zassert_true(cbor_decode_NestedListMap(payload_nested_lm5,
			sizeof(payload_nested_lm5), &listmap, NULL), NULL);
	zassert_equal(2, listmap._NestedListMap_map_count, NULL);
	zassert_false(listmap._NestedListMap_map[0]._map_uint4_present, NULL);
	zassert_true(listmap._NestedListMap_map[1]._map_uint4_present, NULL);
	zassert_false(cbor_decode_NestedListMap(payload_nested_lm6_inv,
			sizeof(payload_nested_lm6_inv), &listmap, NULL), NULL);
	zassert_true(cbor_decode_NestedListMap(payload_nested_lm7,
			sizeof(payload_nested_lm7), &listmap, NULL), NULL);
	zassert_equal(3, listmap._NestedListMap_map_count, NULL);
	zassert_false(listmap._NestedListMap_map[0]._map_uint4_present, NULL);
	zassert_false(listmap._NestedListMap_map[1]._map_uint4_present, NULL);
	zassert_false(listmap._NestedListMap_map[2]._map_uint4_present, NULL);
}

void test_nested_map_list_map(void)
{
	const uint8_t payload_nested_mlm1[] = {MAP(1), LIST(0), END LIST(0), END END};
	const uint8_t payload_nested_mlm2[] = {MAP(1), LIST(0), END LIST(1), MAP(0), END END END};
	const uint8_t payload_nested_mlm3[] = {MAP(1), LIST(0), END LIST(2), MAP(0), END MAP(0), END END END};
	const uint8_t payload_nested_mlm4_inv[] = {MAP(1), LIST(0), END LIST(1), MAP(1), MAP(0), END END END END};
	const uint8_t payload_nested_mlm5[] = {MAP(2), LIST(0), END LIST(0), END LIST(0), END LIST(0), END END};
	const uint8_t payload_nested_mlm6[] = {MAP(3), LIST(0), END LIST(0), END LIST(0), END LIST(0), END LIST(0), END LIST(2), MAP(0), END MAP(0), END END END};
	struct NestedMapListMap maplistmap;

	zassert_true(cbor_decode_NestedMapListMap(payload_nested_mlm1,
			sizeof(payload_nested_mlm1), &maplistmap, NULL), NULL);
	zassert_equal(1, maplistmap._NestedMapListMap_key_count, NULL);
	zassert_equal(0, maplistmap._NestedMapListMap_key[0]._NestedMapListMap_key_map_count, NULL);
	zassert_true(cbor_decode_NestedMapListMap(payload_nested_mlm2,
			sizeof(payload_nested_mlm2), &maplistmap, NULL), NULL);
	zassert_equal(1, maplistmap._NestedMapListMap_key_count, NULL);
	zassert_equal(1, maplistmap._NestedMapListMap_key[0]._NestedMapListMap_key_map_count, NULL);
	zassert_true(cbor_decode_NestedMapListMap(payload_nested_mlm3,
			sizeof(payload_nested_mlm3), &maplistmap, NULL), NULL);
	zassert_equal(1, maplistmap._NestedMapListMap_key_count, NULL);
	zassert_equal(2, maplistmap._NestedMapListMap_key[0]._NestedMapListMap_key_map_count, NULL);
	zassert_false(cbor_decode_NestedMapListMap(payload_nested_mlm4_inv,
			sizeof(payload_nested_mlm4_inv), &maplistmap, NULL), NULL);
	zassert_true(cbor_decode_NestedMapListMap(payload_nested_mlm5,
			sizeof(payload_nested_mlm5), &maplistmap, NULL), NULL);
	zassert_equal(2, maplistmap._NestedMapListMap_key_count, NULL);
	zassert_equal(0, maplistmap._NestedMapListMap_key[0]._NestedMapListMap_key_map_count, NULL);
	zassert_equal(0, maplistmap._NestedMapListMap_key[1]._NestedMapListMap_key_map_count, NULL);
	zassert_true(cbor_decode_NestedMapListMap(payload_nested_mlm6,
			sizeof(payload_nested_mlm6), &maplistmap, NULL), NULL);
	zassert_equal(3, maplistmap._NestedMapListMap_key_count, NULL);
	zassert_equal(0, maplistmap._NestedMapListMap_key[0]._NestedMapListMap_key_map_count, NULL);
	zassert_equal(0, maplistmap._NestedMapListMap_key[1]._NestedMapListMap_key_map_count, NULL);
	zassert_equal(2, maplistmap._NestedMapListMap_key[2]._NestedMapListMap_key_map_count, NULL);
}


void test_range(void)
{
	const uint8_t payload_range1[] = {LIST(3),
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x00,
		END
	};

	const uint8_t payload_range2[] = {LIST(6),
		0x05,
		0x08, 0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x00, 0x0A,
		END
	};

	const uint8_t payload_range3_inv[] = {LIST(4),
		0x06, // outside range
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x00,
		END
	};

	const uint8_t payload_range4_inv[] = {LIST(4),
		0x00,
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x0B, //outside range
		END
	};

	const uint8_t payload_range5[] = {LIST(5),
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x07,
		END
	};

	const uint8_t payload_range6_inv[] = {LIST(4),
		0x67, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x6f, 0x6f, // "hellooo" -> too long
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x07,
		END
	};

	const uint8_t payload_range7_inv[] = {LIST(5),
		0x22,
		0x62, 0x68, 0x65, // "he" -> too short
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x07,
		END
	};

	const uint8_t payload_range8_inv[] = {LIST(5),
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x07, 0x08, 0x18, // Last too large
		END
	};

	const uint8_t payload_range9[] = {0x84,
		0x28,
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x0
	};

	const uint8_t payload_range10_inv[] = {0x84,
		0x25,
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x0
	};

	struct Range output;

	zassert_true(cbor_decode_Range(payload_range1, sizeof(payload_range1),
				&output, NULL), NULL);
	zassert_false(output._Range_optMinus5to5_present, NULL);
	zassert_false(output._Range_optStr3to6_present, NULL);
	zassert_false(output._Range_optMinus9toMinus6excl_present, NULL);
	zassert_equal(1, output._Range_multi8_count, NULL);
	zassert_equal(1, output._Range_multiHello_count, NULL);
	zassert_equal(1, output._Range_multi0to10_count, NULL);
	zassert_equal(0, output._Range_multi0to10[0], NULL);

	zassert_true(cbor_decode_Range(payload_range2, sizeof(payload_range2),
				&output, NULL), NULL);
	zassert_true(output._Range_optMinus5to5_present, NULL);
	zassert_equal(5, output._Range_optMinus5to5, "was %d", output._Range_optMinus5to5);
	zassert_false(output._Range_optStr3to6_present, NULL);
	zassert_false(output._Range_optMinus9toMinus6excl_present, NULL);
	zassert_equal(2, output._Range_multi8_count, NULL);
	zassert_equal(1, output._Range_multiHello_count, NULL);
	zassert_equal(2, output._Range_multi0to10_count, NULL);
	zassert_equal(0, output._Range_multi0to10[0], NULL);
	zassert_equal(10, output._Range_multi0to10[1], NULL);

	zassert_false(cbor_decode_Range(payload_range3_inv, sizeof(payload_range3_inv),
				&output, NULL), NULL);

	zassert_false(cbor_decode_Range(payload_range4_inv, sizeof(payload_range4_inv),
				&output, NULL), NULL);

	zassert_true(cbor_decode_Range(payload_range5, sizeof(payload_range5),
				&output, NULL), NULL);
	zassert_false(output._Range_optMinus5to5_present, NULL);
	zassert_true(output._Range_optStr3to6_present, NULL);
	zassert_equal(5, output._Range_optStr3to6.len, NULL);
	zassert_mem_equal("hello", output._Range_optStr3to6.value, 5, NULL);
	zassert_false(output._Range_optMinus9toMinus6excl_present, NULL);
	zassert_equal(1, output._Range_multi8_count, NULL);
	zassert_equal(2, output._Range_multiHello_count, NULL);
	zassert_equal(1, output._Range_multi0to10_count, NULL);
	zassert_equal(7, output._Range_multi0to10[0], NULL);

	zassert_false(cbor_decode_Range(payload_range6_inv, sizeof(payload_range6_inv),
				&output, NULL), NULL);

	zassert_false(cbor_decode_Range(payload_range7_inv, sizeof(payload_range7_inv),
				&output, NULL), NULL);

	zassert_false(cbor_decode_Range(payload_range8_inv, sizeof(payload_range8_inv),
				&output, NULL), NULL);

	zassert_true(cbor_decode_Range(payload_range9, sizeof(payload_range9),
				&output, NULL), NULL);
	zassert_false(output._Range_optMinus5to5_present, NULL);
	zassert_false(output._Range_optStr3to6_present, NULL);
	zassert_true(output._Range_optMinus9toMinus6excl_present, NULL);
	zassert_equal(-9, output._Range_optMinus9toMinus6excl, NULL);
	zassert_equal(1, output._Range_multi8_count, NULL);
	zassert_equal(1, output._Range_multiHello_count, NULL);
	zassert_equal(1, output._Range_multi0to10_count, NULL);
	zassert_equal(0, output._Range_multi0to10[0], NULL);

	zassert_false(cbor_decode_Range(payload_range10_inv, sizeof(payload_range10_inv),
				&output, NULL), NULL);
}

void test_value_range(void)
{
	const uint8_t payload_value_range1[] = {LIST(6),
		11,
		0x19, 0x03, 0xe7, // 999
		0x29, // -10
		1,
		0x18, 42, // 42
		0x65, 'w', 'o', 'r', 'l', 'd', // "world"
		END
	};

	const uint8_t payload_value_range2[] = {LIST(6),
		0x18, 100, // 100
		0x39, 0x03, 0xe8, // -1001
		0x18, 100, // 100
		0,
		0x18, 42, // 42
		0x65, 'w', 'o', 'r', 'l', 'd', // "world"
		END
	};

	const uint8_t payload_value_range3_inv[] = {LIST(6),
		10,
		0x19, 0x03, 0xe7, // 999
		0x29, // -10
		1,
		0x18, 42, // 42
		0x65, 'w', 'o', 'r', 'l', 'd', // "world"
		END
	};

	const uint8_t payload_value_range4_inv[] = {LIST(6),
		11,
		0x19, 0x03, 0xe8, // 1000
		0x29, // -10
		1,
		0x18, 42, // 42
		0x65, 'w', 'o', 'r', 'l', 'd', // "world"
		END
	};

	const uint8_t payload_value_range5_inv[] = {LIST(6),
		11,
		0x19, 0x03, 0xe7, // 999
		0x2a, // -11
		1,
		0x18, 42, // 42
		0x65, 'w', 'o', 'r', 'l', 'd', // "world"
		END
	};

	const uint8_t payload_value_range6_inv[] = {LIST(6),
		11,
		0x19, 0x03, 0xe7, // 999
		0x29, // -10
		2,
		0x18, 42, // 42
		0x65, 'w', 'o', 'r', 'l', 'd', // "world"
		END
	};

	const uint8_t payload_value_range7_inv[] = {LIST(6),
		11,
		0x19, 0x03, 0xe7, // 999
		0x29, // -10
		1,
		0x18, 43, // 42
		0x65, 'w', 'o', 'r', 'l', 'e', // "worle"
		END
	};

	const uint8_t payload_value_range8_inv[] = {LIST(6),
		11,
		0x19, 0x03, 0xe7, // 999
		0x29, // -10
		1,
		0x18, 42, // 42
		0x66, 'w', 'o', 'r', 'l', 'd', 'd', // "world"
		END
	};

	const uint8_t payload_value_range9_inv[] = {LIST(6),
		11,
		0x19, 0x03, 0xe7, // 999
		0x29, // -10
		1,
		0x18, 42, // 42
		0x64, 'w', 'o', 'r', 'l', // "worl"
		END
	};

	const uint8_t payload_value_range10_inv[] = {LIST(6),
		11,
		0x39, 0x03, 0xe6, // -999
		0x39, // -10
		1,
		0x18, 42, // 42
		0x65, 'w', 'o', 'r', 'l', 'd', // "world"
		END
	};

	const uint8_t payload_value_range11_inv[] = {LIST(6),
		11,
		0x1a, 0x10, 0x00, 0x00, 0x00, // 0x10000000
		0x39, 0x03, 0xe8, // -1001
		1,
		0x18, 42, // 42
		0x65, 'w', 'o', 'r', 'l', 'd', // "world"
		END
	};

	struct ValueRange exp_output_value_range1 = {
		._ValueRange_greater10 = 11,
		._ValueRange_less1000 = 999,
		._ValueRange_greatereqmin10 = -10,
		._ValueRange_lesseq1 = 1,
	};
	struct ValueRange exp_output_value_range2 = {
		._ValueRange_greater10 = 100,
		._ValueRange_less1000 = -1001,
		._ValueRange_greatereqmin10 = 100,
		._ValueRange_lesseq1 = 0,
	};

	struct ValueRange output;
	size_t out_len;

	zassert_true(cbor_decode_ValueRange(payload_value_range1, sizeof(payload_value_range1),
					&output, &out_len), NULL);
	zassert_equal(sizeof(payload_value_range1), out_len, NULL);
	zassert_equal(exp_output_value_range1._ValueRange_greater10,
			output._ValueRange_greater10, NULL);
	zassert_equal(exp_output_value_range1._ValueRange_less1000,
			output._ValueRange_less1000, NULL);
	zassert_equal(exp_output_value_range1._ValueRange_greatereqmin10,
			output._ValueRange_greatereqmin10, NULL);
	zassert_equal(exp_output_value_range1._ValueRange_lesseq1,
			output._ValueRange_lesseq1, NULL);

	zassert_true(cbor_decode_ValueRange(payload_value_range2, sizeof(payload_value_range2),
					&output, &out_len), NULL);
	zassert_equal(sizeof(payload_value_range2), out_len, NULL);
	zassert_equal(exp_output_value_range2._ValueRange_greater10,
			output._ValueRange_greater10, NULL);
	zassert_equal(exp_output_value_range2._ValueRange_less1000,
			output._ValueRange_less1000, NULL);
	zassert_equal(exp_output_value_range2._ValueRange_greatereqmin10,
			output._ValueRange_greatereqmin10, NULL);
	zassert_equal(exp_output_value_range2._ValueRange_lesseq1,
			output._ValueRange_lesseq1, NULL);

	zassert_false(cbor_decode_ValueRange(payload_value_range3_inv,
				sizeof(payload_value_range3_inv), &output, &out_len), NULL);
	zassert_false(cbor_decode_ValueRange(payload_value_range4_inv,
				sizeof(payload_value_range4_inv), &output, &out_len), NULL);
	zassert_false(cbor_decode_ValueRange(payload_value_range5_inv,
				sizeof(payload_value_range5_inv), &output, &out_len), NULL);
	zassert_false(cbor_decode_ValueRange(payload_value_range6_inv,
				sizeof(payload_value_range6_inv), &output, &out_len), NULL);
	zassert_false(cbor_decode_ValueRange(payload_value_range7_inv,
				sizeof(payload_value_range7_inv), &output, &out_len), NULL);
	zassert_false(cbor_decode_ValueRange(payload_value_range8_inv,
				sizeof(payload_value_range8_inv), &output, &out_len), NULL);
	zassert_false(cbor_decode_ValueRange(payload_value_range9_inv,
				sizeof(payload_value_range9_inv), &output, &out_len), NULL);
	zassert_false(cbor_decode_ValueRange(payload_value_range10_inv,
				sizeof(payload_value_range10_inv), &output, &out_len), NULL);
	zassert_false(cbor_decode_ValueRange(payload_value_range11_inv,
				sizeof(payload_value_range11_inv), &output, &out_len), NULL);
}

void test_single(void)
{
	uint8_t payload_single0[] = {0x45, 'h', 'e', 'l', 'l', 'o'};
	uint8_t payload_single1[] = {0x18, 52};
	uint8_t payload_single2_inv[] = {0x18, 53};
	uint8_t payload_single3[] = {9};
	uint8_t payload_single4_inv[] = {10};
	struct zcbor_string result_bstr;
	uint_fast32_t result_int;
	size_t out_len;

	zassert_true(cbor_decode_SingleBstr(payload_single0, sizeof(payload_single0), &result_bstr, &out_len), NULL);
	zassert_equal(sizeof(payload_single0), out_len, NULL);
	zassert_equal(5, result_bstr.len, NULL);
	zassert_mem_equal(result_bstr.value, "hello", result_bstr.len, NULL);
	zassert_false(cbor_decode_SingleBstr(payload_single0, 5, &result_bstr, &out_len), NULL);

	zassert_true(cbor_decode_SingleInt(payload_single1, sizeof(payload_single1), NULL, &out_len), NULL); // Result pointer not needed.
	zassert_equal(sizeof(payload_single1), out_len, NULL);
	zassert_false(cbor_decode_SingleInt(payload_single1, 1, NULL, &out_len), NULL);
	zassert_false(cbor_decode_SingleInt(payload_single2_inv, sizeof(payload_single2_inv), NULL, &out_len), NULL);

	zassert_true(cbor_decode_SingleInt2(payload_single3, sizeof(payload_single3), &result_int, &out_len), NULL);
	zassert_equal(sizeof(payload_single3), out_len, NULL);
	zassert_equal(9, result_int, NULL);
	zassert_false(cbor_decode_SingleInt2(payload_single4_inv, sizeof(payload_single4_inv), &result_int, &out_len), NULL);
}

void test_unabstracted(void)
{
	uint8_t payload_unabstracted0[] = {LIST(2), 0x01, 0x03, END};
	uint8_t payload_unabstracted1[] = {LIST(2), 0x02, 0x04, END};
	uint8_t payload_unabstracted2_inv[] = {LIST(2), 0x03, 0x03, END};
	uint8_t payload_unabstracted3_inv[] = {LIST(2), 0x01, 0x01, END};
	struct Unabstracted result_unabstracted;
	size_t out_len;

	zassert_true(cbor_decode_Unabstracted(payload_unabstracted0,
					sizeof(payload_unabstracted0),
					&result_unabstracted, &out_len), NULL);
	zassert_equal(result_unabstracted._Unabstracted_unabstractedunion1_choice,
			_Unabstracted_unabstractedunion1_choice1, NULL);
	zassert_equal(result_unabstracted._Unabstracted_unabstractedunion2_choice,
			_Unabstracted_unabstractedunion2_uint3, NULL);
	zassert_equal(sizeof(payload_unabstracted0), out_len, NULL);

	zassert_true(cbor_decode_Unabstracted(payload_unabstracted1,
					sizeof(payload_unabstracted1),
					&result_unabstracted, &out_len), NULL);
	zassert_equal(result_unabstracted._Unabstracted_unabstractedunion1_choice,
			_Unabstracted_unabstractedunion1_choice2, NULL);
	zassert_equal(result_unabstracted._Unabstracted_unabstractedunion2_choice,
			_Unabstracted_unabstractedunion2_choice4, NULL);
	zassert_equal(sizeof(payload_unabstracted1), out_len, NULL);

	zassert_false(cbor_decode_Unabstracted(payload_unabstracted2_inv,
					sizeof(payload_unabstracted2_inv),
					&result_unabstracted, &out_len), NULL);
	zassert_false(cbor_decode_Unabstracted(payload_unabstracted3_inv,
					sizeof(payload_unabstracted3_inv),
					&result_unabstracted, &out_len), NULL);
}

void test_quantity_range(void)
{
	uint8_t payload_qty_range1[] = {0xF5, 0xF5, 0xF5};
	uint8_t payload_qty_range2_inv[] = {0xF5, 0xF5};
	uint8_t payload_qty_range3[] = {0xF6, 0xF6, 0xF6, 0xF6, 0xF5, 0xF5, 0xF5, 0xF5, 0xF5, 0xF5};
	uint8_t payload_qty_range4_inv[] = {0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF5, 0xF5, 0xF5};
	struct QuantityRange result_qty_range;
	size_t out_len;

	zassert_true(cbor_decode_QuantityRange(payload_qty_range1,
					sizeof(payload_qty_range1),
					&result_qty_range, &out_len), NULL);
	zassert_equal(0, result_qty_range._QuantityRange_upto4nils_count, NULL);
	zassert_equal(3, result_qty_range._QuantityRange_from3true_count, NULL);
	zassert_equal(sizeof(payload_qty_range1), out_len, NULL);

	zassert_false(cbor_decode_QuantityRange(payload_qty_range2_inv,
					sizeof(payload_qty_range2_inv),
					&result_qty_range, &out_len), NULL);

	zassert_true(cbor_decode_QuantityRange(payload_qty_range3,
					sizeof(payload_qty_range3),
					&result_qty_range, &out_len), NULL);
	zassert_equal(4, result_qty_range._QuantityRange_upto4nils_count, NULL);
	zassert_equal(6, result_qty_range._QuantityRange_from3true_count, NULL);
	zassert_equal(sizeof(payload_qty_range3), out_len, NULL);

	zassert_false(cbor_decode_QuantityRange(payload_qty_range4_inv,
					sizeof(payload_qty_range4_inv),
					&result_qty_range, &out_len), NULL);
}

void test_doublemap(void)
{
	uint8_t payload_doublemap0[] = {0xA2, 0x01, 0xA1, 0x01, 0x01, 0x02, 0xA1, 0x02, 0x02};
	uint8_t payload_doublemap1_inv[] = {0xA2, 0x01, 0xA1, 0x01, 0x01, 0x02, 0xA1, 0x03, 0x02};
	struct DoubleMap result_doublemap;
	size_t out_len;

	zassert_true(cbor_decode_DoubleMap(payload_doublemap0,
					sizeof(payload_doublemap0),
					&result_doublemap, &out_len), NULL);
	zassert_equal(result_doublemap._DoubleMap_uintmap_count, 2, NULL);
	zassert_equal(result_doublemap._DoubleMap_uintmap[0]._DoubleMap_uintmap_key, 1, NULL);
	zassert_true(result_doublemap._DoubleMap_uintmap[0]._DoubleMap_uintmap__MyKeys._MyKeys_uint1int_present, NULL);
	zassert_equal(result_doublemap._DoubleMap_uintmap[0]._DoubleMap_uintmap__MyKeys._MyKeys_uint1int._MyKeys_uint1int, 1, NULL);
	zassert_false(result_doublemap._DoubleMap_uintmap[0]._DoubleMap_uintmap__MyKeys._MyKeys_uint2int_present, NULL);
	zassert_equal(result_doublemap._DoubleMap_uintmap[1]._DoubleMap_uintmap_key, 2, NULL);
	zassert_false(result_doublemap._DoubleMap_uintmap[1]._DoubleMap_uintmap__MyKeys._MyKeys_uint1int_present, NULL);
	zassert_true(result_doublemap._DoubleMap_uintmap[1]._DoubleMap_uintmap__MyKeys._MyKeys_uint2int_present, NULL);
	zassert_equal(result_doublemap._DoubleMap_uintmap[1]._DoubleMap_uintmap__MyKeys._MyKeys_uint2int._MyKeys_uint2int, 2, NULL);

	zassert_false(cbor_decode_DoubleMap(payload_doublemap1_inv,
					sizeof(payload_doublemap1_inv),
					&result_doublemap, &out_len), NULL);
}

void test_main(void)
{
	ztest_test_suite(cbor_decode_test5,
			 ztest_unit_test(test_numbers),
			 ztest_unit_test(test_numbers2),
			 ztest_unit_test(test_strings),
			 ztest_unit_test(test_primitives),
			 ztest_unit_test(test_string_overflow),
			 ztest_unit_test(test_optional),
			 ztest_unit_test(test_union),
			 ztest_unit_test(test_levels),
			 ztest_unit_test(test_map),
			 ztest_unit_test(test_empty_map),
			 ztest_unit_test(test_nested_list_map),
			 ztest_unit_test(test_nested_map_list_map),
			 ztest_unit_test(test_range),
			 ztest_unit_test(test_value_range),
			 ztest_unit_test(test_single),
			 ztest_unit_test(test_unabstracted),
			 ztest_unit_test(test_quantity_range),
			 ztest_unit_test(test_unabstracted),
			 ztest_unit_test(test_doublemap)
	);
	ztest_run_test_suite(cbor_decode_test5);
}
