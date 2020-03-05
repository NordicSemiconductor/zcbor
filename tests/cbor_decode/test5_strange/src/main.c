/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include "strange_decode.h"

void test_numbers(void)
{
	const uint8_t payload_numbers1[] = {
		0x8A, // List start
			0x01, // 1
			0x21, // -2
			0x05, // 5
			0x19, 0x01, 0x00, // 256
			0x1A, 0x01, 0x02, 0x03, 0x04, // 0x01020304
			0x39, 0x13, 0x87, // -5000
			0x1A, 0xEE, 0x6B, 0x28, 0x00, // 4000000000
			0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -2^31
			0x00, // 0
			0x01 // 1
	};

	Numbers_t numbers;
	zassert_false(cbor_decode_Numbers(payload_numbers1,
		sizeof(payload_numbers1) + 1, &numbers, true), NULL); // Payload too large
	zassert_true(cbor_decode_Numbers(payload_numbers1,
		sizeof(payload_numbers1), &numbers, true), NULL);

	zassert_equal(5, numbers._Numbers_fourtoten, NULL);
	zassert_equal(256, numbers._Numbers_twobytes, NULL);
	zassert_equal(0x01020304, numbers._Numbers_onetofourbytes, NULL);
	zassert_equal(-5000, numbers._Numbers_minusfivektoplustwohunred, "%d", numbers._Numbers_minusfivektoplustwohunred);
	zassert_equal(-2147483648, numbers._Numbers_negint, NULL);
	zassert_equal(0, numbers._Numbers_posint, NULL);
	zassert_equal(1, numbers._Numbers_integer, NULL);
}


void test_strings(void)
{
	const uint8_t payload_strings1[] = {
		0x86,
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
		0x78, 0x1E, // 30 bytes
		0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,
		0x58, 26, // Numbers (len: 26)
			0x8A, // List start
				0x01, // 1
				0x21, // -2
				0x05, // 5
				0x19, 0xFF, 0xFF, // 0xFFFF
				0x18, 0x18, // 24
				0x00, // 0
				0x1A, 0xEE, 0x6B, 0x28, 0x00, // 4000000000
				0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -2^31
				0x1A, 0xFF, 0xFF, 0xFF, 0xFF, // 0xFFFFFFFF
				0x09, // 9
		0x4F, // Primitives (len: 15)
			0x84, // List start
				0xF5, // True
				0xF4, // False
				0xF4, // False
				0xF6, // Nil
			0x84, // List start
				0xF5, // True
				0xF4, // False
				0xF5, // True
				0xF6, // Nil
			0x84, // List start
				0xF5, // True
				0xF4, // False
				0xF4, // False
				0xF6, // Nil
		0x59, 0x01, 0x63, // Strings (len: 355)
			0x85,
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
			0x6A, // 10 bytes
			0,1,2,3,4,5,6,7,8,9,
			0x58, 26, // Numbers (len: 26)
				0x8A, // List start
					0x01, // 1
					0x21, // -2
					0x05, // 5
					0x19, 0xFF, 0xFF, // 0xFFFF
					0x18, 0x18, // 24
					0x00, // 0
					0x1A, 0xEE, 0x6B, 0x28, 0x00, // 4000000000
					0x3A, 0x7F, 0xFF, 0xFF, 0xFF, // -2^31
					0x1A, 0xFF, 0xFF, 0xFF, 0xFF, // 0xFFFFFFFF
					0x29, // -10
			0x45, // Primitives (len: 5)
				0x84, // List start
					0xF5, // True
					0xF4, // False
					0xF4, // False
					0xF6, // Nil
	};

	Strings_t strings1;
	Strings_t strings2;
	Numbers_t numbers1;
	Numbers_t numbers2;
	zassert_true(cbor_decode_Strings(payload_strings1,
		sizeof(payload_strings1), &strings1, true), NULL);
	zassert_true(strings1._Strings_optCborStrings_present, NULL);
	zassert_true(cbor_decode_Strings(strings1._Strings_optCborStrings.value,
		strings1._Strings_optCborStrings.len, &strings2, true), NULL);
	zassert_true(cbor_decode_Numbers(strings1._Strings_cborNumbers.value,
		strings1._Strings_cborNumbers.len, &numbers1, true), NULL);
	zassert_true(cbor_decode_Numbers(strings2._Strings_cborNumbers.value,
		strings2._Strings_cborNumbers.len, &numbers2, true), NULL);

	zassert_equal(300, strings1._Strings_threehundrebytebstr.len, NULL);
	zassert_equal(0, strings1._Strings_threehundrebytebstr.value[0], NULL);
	zassert_equal(9, strings1._Strings_threehundrebytebstr.value[299], NULL);
	zassert_equal(30, strings1._Strings_tentothirtybytetstr.len, NULL);
	zassert_equal(26, strings1._Strings_cborNumbers.len, NULL);
	zassert_equal(3, strings1._Strings_cborseqPrimitives_cbor_count, NULL);
	zassert_false(strings1._Strings_cborseqPrimitives_cbor[0]._Primitives_boolval, NULL);
	zassert_true(strings1._Strings_cborseqPrimitives_cbor[1]._Primitives_boolval, NULL);
	zassert_false(strings1._Strings_cborseqPrimitives_cbor[2]._Primitives_boolval, NULL);

	zassert_equal(300, strings2._Strings_threehundrebytebstr.len, NULL);
	zassert_equal(10, strings2._Strings_tentothirtybytetstr.len, NULL);
	zassert_equal(5, numbers2._Numbers_fourtoten, NULL);
	zassert_equal(0xFFFF, numbers2._Numbers_twobytes, NULL);
	zassert_equal(24, numbers2._Numbers_onetofourbytes, NULL);
	zassert_equal(0, numbers2._Numbers_minusfivektoplustwohunred, NULL);
	zassert_equal(-2147483648, numbers2._Numbers_negint, NULL);
	zassert_equal(0xFFFFFFFF, numbers2._Numbers_posint, NULL);
	zassert_equal(-10, numbers2._Numbers_integer, NULL);
	zassert_equal(1, strings2._Strings_cborseqPrimitives_cbor_count, NULL);
	zassert_false(strings2._Strings_cborseqPrimitives_cbor[0]._Primitives_boolval, NULL);
}

void test_optional(void)
{
	const uint8_t payload_optional1[] = {
		0x83 /* List start */, 0xF4 /* False */, 0x02, 0x03,
	};
	const uint8_t payload_optional2[] = {
		0x82 /* List start */, 0xF4 /* False */, 0x03,
	};
	const uint8_t payload_optional3_inv[] = {
		0x82 /* List start */, 0xF4 /* False */, 0x02,
	};
	const uint8_t payload_optional4[] = {
		0x83 /* List start */, 0xF4 /* False */, 0x02, 0x01,
	};
	const uint8_t payload_optional5[] = {
		0x83 /* List start */, 0xF5 /* True */, 0x02, 0x02,
	};
	const uint8_t payload_optional6[] = {
		0x84 /* List start */, 0xF5 /* True */, 0xF4 /* False */, 0x02, 0x02,
	};

	Optional_t optional;
	zassert_true(cbor_decode_Optional(payload_optional1,
			sizeof(payload_optional1), &optional, true), NULL);
	zassert_false(optional._Optional_boolval, NULL);
	zassert_false(optional._Optional_optbool_present, NULL);
	zassert_true(optional._Optional_opttwo_present, NULL);
	zassert_equal(3, optional._Optional_manduint, NULL);

	zassert_true(cbor_decode_Optional(payload_optional2,
			sizeof(payload_optional2), &optional, true), NULL);
	zassert_false(optional._Optional_boolval, NULL);
	zassert_false(optional._Optional_optbool_present, NULL);
	zassert_false(optional._Optional_opttwo_present, NULL);
	zassert_equal(3, optional._Optional_manduint, NULL);

	zassert_false(cbor_decode_Optional(payload_optional3_inv,
			sizeof(payload_optional3_inv), &optional, true), NULL);

	zassert_true(cbor_decode_Optional(payload_optional4,
			sizeof(payload_optional4), &optional, true), NULL);
	zassert_false(optional._Optional_boolval, NULL);
	zassert_false(optional._Optional_optbool_present, NULL);
	zassert_true(optional._Optional_opttwo_present, NULL);
	zassert_equal(1, optional._Optional_manduint, NULL);

	zassert_true(cbor_decode_Optional(payload_optional5,
			sizeof(payload_optional5), &optional, true), NULL);
	zassert_true(optional._Optional_boolval, NULL);
	zassert_false(optional._Optional_optbool_present, NULL);
	zassert_true(optional._Optional_opttwo_present, NULL);
	zassert_equal(2, optional._Optional_manduint, NULL);

	zassert_true(cbor_decode_Optional(payload_optional6,
			sizeof(payload_optional6), &optional, true), NULL);
	zassert_true(optional._Optional_boolval, NULL);
	zassert_true(optional._Optional_optbool_present, NULL);
	zassert_false(optional._Optional_optbool, NULL);
	zassert_true(optional._Optional_opttwo_present, NULL);
	zassert_equal(2, optional._Optional_manduint, NULL);
}

void test_union(void)
{
	const uint8_t payload_union1[] = {0x01, 0x21};
	const uint8_t payload_union2[] = {0x03, 0x23};
	const uint8_t payload_union3[] = {0x03, 0x04};
	const uint8_t payload_union4[] = {
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f // "hello"
	};
	const uint8_t payload_union6[] = {0x03, 0x23, 0x03, 0x23, 0x03, 0x23};
	const uint8_t payload_union7_inv[] = {0x03, 0x23, 0x03, 0x04};
	const uint8_t payload_union8_inv[] = {0x03, 0x23, 0x03};
	const uint8_t payload_union9_inv[] = {0x03, 0x04, 0x03, 0x04};

	_Union_t _union;
	zassert_true(cbor_decode_Union(payload_union1, sizeof(payload_union1),
				&_union, true), NULL);
	zassert_equal(_Union__Group, _union._Union_choice, NULL);

	zassert_true(cbor_decode_Union(payload_union2, sizeof(payload_union2),
				&_union, true), NULL);
	zassert_equal(_Union__MultiGroup, _union._Union_choice, NULL);
	zassert_equal(1, _union._Union__MultiGroup._MultiGroup_count, "was %d\r\n", _union._Union__MultiGroup._MultiGroup_count);

	zassert_true(cbor_decode_Union(payload_union3, sizeof(payload_union3),
				&_union, true), NULL);
	zassert_equal(_Union__uint3, _union._Union_choice, NULL);

	zassert_true(cbor_decode_Union(payload_union4, sizeof(payload_union4),
				&_union, true), NULL);
	zassert_equal(_Union_hello_tstr, _union._Union_choice, NULL);

	zassert_true(cbor_decode_Union(payload_union6, sizeof(payload_union6),
				&_union, true), NULL);
	zassert_equal(_Union__MultiGroup, _union._Union_choice, NULL);
	zassert_equal(3, _union._Union__MultiGroup._MultiGroup_count, NULL);

	zassert_false(cbor_decode_Union(payload_union7_inv, sizeof(payload_union7_inv),
				&_union, true), NULL);
	zassert_false(cbor_decode_Union(payload_union8_inv, sizeof(payload_union8_inv),
				&_union, true), NULL);
	zassert_false(cbor_decode_Union(payload_union9_inv, sizeof(payload_union9_inv),
				&_union, true), NULL);
}

void test_levels(void)
{
	const uint8_t payload_levels1[] = {
		0x81, // Level1
		0x82, // Level2
		0x84, // Level3 no 1
		0x81, 0x00, // Level4 no 1
		0x81, 0x00, // Level4 no 2
		0x81, 0x00, // Level4 no 3
		0x81, 0x00, // Level4 no 4
		0x84, // Level3 no 2
		0x81, 0x00, // Level4 no 1
		0x81, 0x00, // Level4 no 2
		0x81, 0x00, // Level4 no 3
		0x81, 0x00, // Level4 no 4
	};

	Level2_t level1;
	zassert_true(cbor_decode_Level1(payload_levels1,
		sizeof(payload_levels1), &level1, true), NULL);

	zassert_equal(2, level1._Level2__Level3_count, NULL);
	zassert_equal(4, level1._Level2__Level3[0]._Level3__Level4_count, NULL);
	zassert_equal(4, level1._Level2__Level3[1]._Level3__Level4_count, NULL);
}


void test_map(void)
{
	const uint8_t payload_map1[] = {
		0xa4, 0x82, 0x05, 0x06, 0xF4, // [5,6] => false
		0x07, 0x01, // 7 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
		0xf6, 0x40, // nil => ""
	};
	const uint8_t payload_map2_inv[] = {
		0xa4, 0x82, 0x05, 0x06, 0xF4, // [5,6] => false
		0x07, 0x01, // 7 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
	};
	const uint8_t payload_map3[] = {
		0xa5, 0x82, 0x05, 0x06, 0xF5, // [5,6] => true
		0x07, 0x01, // 7 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
		0xf6, 0x40, // nil => ""
		0xf6, 0x40, // nil => ""
	};
	const uint8_t payload_map4_inv[] = {
		0xa6, 0x82, 0x05, 0x06, 0xF4, // [5,6] => false
		0x07, 0x01, // 7 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
		0xf6, 0x40, // nil => ""
		0xf6, 0x40, // nil => ""
	};
	const uint8_t payload_map5[] = {
		0xa4, 0x82, 0x05, 0x06, 0xF4, // [5,6] => false
		0x27, 0x01, // -8 => 1
		0xf6, 0x45, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // nil => "hello"
		0xf6, 0x40 // nil => ""
	};

	Map_t map;

	zassert_true(cbor_decode_Map(payload_map1, sizeof(payload_map1),
			&map, true), NULL);
	zassert_false(map._Map_key, NULL);
	zassert_equal(_Map_union_uint7uint, map._Map_union_choice, NULL);
	zassert_equal(1, map._Map_union_uint7uint, NULL);
	zassert_equal(2, map._Map_twotothree_count, NULL);
	zassert_equal(5, map._Map_twotothree[0]._Map_twotothree.len, NULL);
	zassert_mem_equal("hello", map._Map_twotothree[0]._Map_twotothree.value,
			5, NULL);
	zassert_equal(0, map._Map_twotothree[1]._Map_twotothree.len, NULL);

	zassert_false(cbor_decode_Map(payload_map2_inv, sizeof(payload_map2_inv),
			&map, true), NULL);

	zassert_true(cbor_decode_Map(payload_map3, sizeof(payload_map3),
			&map, true), NULL);
	zassert_true(map._Map_key, NULL);
	zassert_equal(_Map_union_uint7uint, map._Map_union_choice, NULL);
	zassert_equal(1, map._Map_union_uint7uint, NULL);
	zassert_equal(3, map._Map_twotothree_count, NULL);
	zassert_equal(5, map._Map_twotothree[0]._Map_twotothree.len, NULL);
	zassert_mem_equal("hello", map._Map_twotothree[0]._Map_twotothree.value,
			5, NULL);
	zassert_equal(0, map._Map_twotothree[1]._Map_twotothree.len, NULL);
	zassert_equal(0, map._Map_twotothree[2]._Map_twotothree.len, NULL);

	zassert_false(cbor_decode_Map(payload_map4_inv, sizeof(payload_map4_inv),
			&map, true), NULL);

	zassert_true(cbor_decode_Map(payload_map5, sizeof(payload_map5),
			&map, true), NULL);
	zassert_false(map._Map_key, NULL);
	zassert_equal(_Map_union_nintuint, map._Map_union_choice, NULL);
	zassert_equal(1, map._Map_union_nintuint, NULL);
	zassert_equal(2, map._Map_twotothree_count, NULL);
}

void test_nested_list_map(void)
{
	const uint8_t payload_nested_lm1[] = {0x80};
	const uint8_t payload_nested_lm2[] = {0x81, 0xa0};
	const uint8_t payload_nested_lm3[] = {0x81, 0xa1, 0x01, 0x04};
	const uint8_t payload_nested_lm4_inv[] = {0x81, 0xa2, 0x01, 0x04, 0x1, 0x4};
	const uint8_t payload_nested_lm5[] = {0x82, 0xa0, 0xa1, 0x01, 0x04};
	const uint8_t payload_nested_lm6_inv[] = {0x82, 0xa0, 0xa1, 0x04};
	const uint8_t payload_nested_lm7[] = {0x83, 0xa0, 0xa0, 0xa0};
	NestedListMap_t listmap;

	zassert_true(cbor_decode_NestedListMap(payload_nested_lm1,
			sizeof(payload_nested_lm1), &listmap, true), NULL);
	zassert_equal(0, listmap._NestedListMap_map_count, NULL);
	zassert_true(cbor_decode_NestedListMap(payload_nested_lm2,
			sizeof(payload_nested_lm2), &listmap, true), NULL);
	zassert_equal(1, listmap._NestedListMap_map_count, NULL);
	zassert_false(listmap._NestedListMap_map[0]._NestedListMap_map_uint4_present, NULL);
	zassert_true(cbor_decode_NestedListMap(payload_nested_lm3,
			sizeof(payload_nested_lm3), &listmap, true), NULL);
	zassert_equal(1, listmap._NestedListMap_map_count, NULL);
	zassert_true(listmap._NestedListMap_map[0]._NestedListMap_map_uint4_present, NULL);
	zassert_equal(1, listmap._NestedListMap_map[0]._NestedListMap_map_uint4_present, NULL);
	zassert_false(cbor_decode_NestedListMap(payload_nested_lm4_inv,
			sizeof(payload_nested_lm4_inv), &listmap, true), NULL);
	zassert_true(cbor_decode_NestedListMap(payload_nested_lm5,
			sizeof(payload_nested_lm5), &listmap, true), NULL);
	zassert_equal(2, listmap._NestedListMap_map_count, NULL);
	zassert_false(listmap._NestedListMap_map[0]._NestedListMap_map_uint4_present, NULL);
	zassert_true(listmap._NestedListMap_map[1]._NestedListMap_map_uint4_present, NULL);
	zassert_false(cbor_decode_NestedListMap(payload_nested_lm6_inv,
			sizeof(payload_nested_lm6_inv), &listmap, true), NULL);
	zassert_true(cbor_decode_NestedListMap(payload_nested_lm7,
			sizeof(payload_nested_lm7), &listmap, true), NULL);
	zassert_equal(3, listmap._NestedListMap_map_count, NULL);
	zassert_false(listmap._NestedListMap_map[0]._NestedListMap_map_uint4_present, NULL);
	zassert_false(listmap._NestedListMap_map[1]._NestedListMap_map_uint4_present, NULL);
	zassert_false(listmap._NestedListMap_map[2]._NestedListMap_map_uint4_present, NULL);
}

void test_nested_map_list_map(void)
{
	const uint8_t payload_nested_mlm1[] = {0xa1, 0x80, 0x80};
	const uint8_t payload_nested_mlm2[] = {0xa1, 0x80, 0x81, 0xa0};
	const uint8_t payload_nested_mlm3[] = {0xa1, 0x80, 0x82, 0xa0, 0xa0};
	const uint8_t payload_nested_mlm4_inv[] = {0xa1, 0x80, 0x81, 0xa1, 0xa0};
	const uint8_t payload_nested_mlm5[] = {0xa2, 0x80, 0x80, 0x80, 0x80};
	const uint8_t payload_nested_mlm6[] = {0xa3, 0x80, 0x80, 0x80, 0x80, 0x80, 0x82, 0xa0, 0xa0};
	NestedMapListMap_t maplistmap;

	zassert_true(cbor_decode_NestedMapListMap(payload_nested_mlm1,
			sizeof(payload_nested_mlm1), &maplistmap, true), NULL);
	zassert_equal(1, maplistmap._NestedMapListMap_key_count, NULL);
	zassert_equal(0, maplistmap._NestedMapListMap_key[0]._NestedMapListMap_key_map_count, NULL);
	zassert_true(cbor_decode_NestedMapListMap(payload_nested_mlm2,
			sizeof(payload_nested_mlm2), &maplistmap, true), NULL);
	zassert_equal(1, maplistmap._NestedMapListMap_key_count, NULL);
	zassert_equal(1, maplistmap._NestedMapListMap_key[0]._NestedMapListMap_key_map_count, NULL);
	zassert_true(cbor_decode_NestedMapListMap(payload_nested_mlm3,
			sizeof(payload_nested_mlm3), &maplistmap, true), NULL);
	zassert_equal(1, maplistmap._NestedMapListMap_key_count, NULL);
	zassert_equal(2, maplistmap._NestedMapListMap_key[0]._NestedMapListMap_key_map_count, NULL);
	zassert_false(cbor_decode_NestedMapListMap(payload_nested_mlm4_inv,
			sizeof(payload_nested_mlm4_inv), &maplistmap, true), NULL);
	zassert_true(cbor_decode_NestedMapListMap(payload_nested_mlm5,
			sizeof(payload_nested_mlm5), &maplistmap, true), NULL);
	zassert_equal(2, maplistmap._NestedMapListMap_key_count, NULL);
	zassert_equal(0, maplistmap._NestedMapListMap_key[0]._NestedMapListMap_key_map_count, NULL);
	zassert_equal(0, maplistmap._NestedMapListMap_key[1]._NestedMapListMap_key_map_count, NULL);
	zassert_true(cbor_decode_NestedMapListMap(payload_nested_mlm6,
			sizeof(payload_nested_mlm6), &maplistmap, true), NULL);
	zassert_equal(3, maplistmap._NestedMapListMap_key_count, NULL);
	zassert_equal(0, maplistmap._NestedMapListMap_key[0]._NestedMapListMap_key_map_count, NULL);
	zassert_equal(0, maplistmap._NestedMapListMap_key[1]._NestedMapListMap_key_map_count, NULL);
	zassert_equal(2, maplistmap._NestedMapListMap_key[2]._NestedMapListMap_key_map_count, NULL);
}


void test_range(void)
{
	const uint8_t payload_range1[] = {0x83,
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x0
	};

	const uint8_t payload_range2[] = {0x86,
		0x05,
		0x08, 0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x00, 0x0A
	};

	const uint8_t payload_range3_inv[] = {0x84,
		0x06, // outside range
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x00
	};

	const uint8_t payload_range4_inv[] = {0x84,
		0x00,
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x0B //outside range
	};

	const uint8_t payload_range5[] = {0x85,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x07
	};

	const uint8_t payload_range6_inv[] = {0x84,
		0x67, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x6f, 0x6f, // "hellooo" -> too long
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x07
	};

	const uint8_t payload_range7_inv[] = {0x85,
		0x22,
		0x62, 0x68, 0x65, // "he" -> too short
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x07
	};

	const uint8_t payload_range8_inv[] = {0x85,
		0x08,
		0x65, 0x68, 0x65, 0x6c, 0x6c, 0x6f, // "hello"
		0x07, 0x08, 0x18 // Last too large
	};

	Range_t output;

	zassert_true(cbor_decode_Range(payload_range1, sizeof(payload_range1),
				&output, true), NULL);
	zassert_false(output._Range_optMinus5to5_present, NULL);
	zassert_false(output._Range_optStr3to6_present, NULL);
	zassert_equal(1, output._Range_multi8_count, NULL);
	zassert_equal(1, output._Range_multiHello_count, NULL);
	zassert_equal(1, output._Range_multi0to10_count, NULL);
	zassert_equal(0, output._Range_multi0to10[0], NULL);

	zassert_true(cbor_decode_Range(payload_range2, sizeof(payload_range2),
				&output, true), NULL);
	zassert_true(output._Range_optMinus5to5_present, NULL);
	zassert_equal(5, output._Range_optMinus5to5, "was %d", output._Range_optMinus5to5);
	zassert_false(output._Range_optStr3to6_present, NULL);
	zassert_equal(2, output._Range_multi8_count, NULL);
	zassert_equal(1, output._Range_multiHello_count, NULL);
	zassert_equal(2, output._Range_multi0to10_count, NULL);
	zassert_equal(0, output._Range_multi0to10[0], NULL);
	zassert_equal(10, output._Range_multi0to10[1], NULL);

	zassert_false(cbor_decode_Range(payload_range3_inv, sizeof(payload_range3_inv),
				&output, true), NULL);

	zassert_false(cbor_decode_Range(payload_range4_inv, sizeof(payload_range4_inv),
				&output, true), NULL);

	zassert_true(cbor_decode_Range(payload_range5, sizeof(payload_range5),
				&output, true), NULL);
	zassert_false(output._Range_optMinus5to5_present, NULL);
	zassert_true(output._Range_optStr3to6_present, NULL);
	zassert_equal(5, output._Range_optStr3to6.len, NULL);
	zassert_mem_equal("hello", output._Range_optStr3to6.value, 5, NULL);
	zassert_equal(1, output._Range_multi8_count, NULL);
	zassert_equal(2, output._Range_multiHello_count, NULL);
	zassert_equal(1, output._Range_multi0to10_count, NULL);
	zassert_equal(7, output._Range_multi0to10[0], NULL);

	zassert_false(cbor_decode_Range(payload_range6_inv, sizeof(payload_range6_inv),
				&output, true), NULL);

	zassert_false(cbor_decode_Range(payload_range7_inv, sizeof(payload_range7_inv),
				&output, true), NULL);

	zassert_false(cbor_decode_Range(payload_range8_inv, sizeof(payload_range8_inv),
				&output, true), NULL);
}


void test_main(void)
{
	ztest_test_suite(cbor_decode_test5,
			 ztest_unit_test(test_numbers),
			 ztest_unit_test(test_strings),
			 ztest_unit_test(test_optional),
			 ztest_unit_test(test_union),
			 ztest_unit_test(test_levels),
			 ztest_unit_test(test_map),
			 ztest_unit_test(test_nested_list_map),
			 ztest_unit_test(test_nested_map_list_map),
			 ztest_unit_test(test_range)
	);
	ztest_run_test_suite(cbor_decode_test5);
}
