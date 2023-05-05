/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <zcbor_common.h>
#include <zcbor_encode.h>
#include <zcbor_print.h>
#include "unordered_map_decode.h"
#include <common_test.h>



ZTEST(cbor_decode_testA, test_unordered_map1)
{
	const uint8_t payload_unordered_map1_1[] = {
		MAP(7),
		3, 0x40,
		0x41, 0, LIST(3), 1, 2, 3, END
		2, 0x63, 't', 'w', 'o',
		0x63, 'b', 'a', 'z', 0x63, 'b', 'o', 'z',
		0x41, 2, LIST(0), END
		1, 0x63, 'o', 'n', 'e',
		0x63, 'f', 'o', 'o', 0x63, 'b', 'a', 'r',
		END
	};

	const uint8_t payload_unordered_map1_2[] = {
		MAP(5),
		2, 0x63, 't', 'w', 'o',
		3, 0x40,
		1, 0x63, 'o', 'n', 'e',
		0x63, 'b', 'a', 'z', 0x63, 'b', 'o', 'z',
		0x63, 'f', 'o', 'o', 0x63, 'b', 'a', 'r',
		END
	};

	const uint8_t payload_unordered_map1_inv3[] = {
		MAP(5),
		2, 0x63, 't', 'w', 'o',
		3, 0x40,
		1, 0x63, 'o', 'n', 'f' /* here */,
		0x63, 'b', 'a', 'z', 0x63, 'b', 'o', 'z',
		0x63, 'f', 'o', 'o', 0x63, 'b', 'a', 'r',
		END
	};

	const uint8_t payload_unordered_map1_inv4[] = {
		MAP(6),
		3, 0x40,
		0x63, 'b', 'a', 'z', 0x63, 'b', 'o', 'z',
		0x41, 0, LIST(3), 1, 2, 0x40 /* here */, END
		0x63, 'f', 'o', 'o', 0x63, 'b', 'a', 'r',
		1, 0x63, 'o', 'n', 'e',
		2, 0x63, 't', 'w', 'o',
		END
	};

	struct UnorderedMap1 unordered_map1;

	int err = cbor_decode_UnorderedMap1(payload_unordered_map1_1, sizeof(payload_unordered_map1_1), &unordered_map1, NULL);
	zassert_equal(ZCBOR_SUCCESS, err, "%s %d\n", zcbor_error_str(err), err);
	zassert_equal(3, unordered_map1.UnorderedMap1_intbstr_key);
	zassert_equal(0, unordered_map1.UnorderedMap1_intbstr.len);
	zassert_equal(2, unordered_map1.UnorderedMap1_intlist_m_count);
	zassert_equal(1, unordered_map1.UnorderedMap1_intlist_m[0].UnorderedMap1_intlist_m_key.len);
	zassert_equal(2, unordered_map1.UnorderedMap1_intlist_m[0].UnorderedMap1_intlist_m_key.value[0]);
	zassert_equal(0, unordered_map1.UnorderedMap1_intlist_m[0].UnorderedMap1_intlist_m.intlist_int_count);
	zassert_equal(1, unordered_map1.UnorderedMap1_intlist_m[1].UnorderedMap1_intlist_m_key.len);
	zassert_equal(0, unordered_map1.UnorderedMap1_intlist_m[1].UnorderedMap1_intlist_m_key.value[0]);
	zassert_equal(3, unordered_map1.UnorderedMap1_intlist_m[1].UnorderedMap1_intlist_m.intlist_int_count);
	zassert_equal(1, unordered_map1.UnorderedMap1_intlist_m[1].UnorderedMap1_intlist_m.intlist_int[0]);
	zassert_equal(2, unordered_map1.UnorderedMap1_intlist_m[1].UnorderedMap1_intlist_m.intlist_int[1]);
	zassert_equal(3, unordered_map1.UnorderedMap1_intlist_m[1].UnorderedMap1_intlist_m.intlist_int[2]);

	err = cbor_decode_UnorderedMap1(payload_unordered_map1_2, sizeof(payload_unordered_map1_2), &unordered_map1, NULL);
	zassert_equal(ZCBOR_SUCCESS, err, "%s %d\n", zcbor_error_str(err), err);
	zassert_equal(3, unordered_map1.UnorderedMap1_intbstr_key);
	zassert_equal(0, unordered_map1.UnorderedMap1_intbstr.len);
	zassert_equal(0, unordered_map1.UnorderedMap1_intlist_m_count);

	err = cbor_decode_UnorderedMap1(payload_unordered_map1_inv3, sizeof(payload_unordered_map1_inv3), &unordered_map1, NULL);
	zassert_equal(ZCBOR_ERR_WRONG_VALUE, err, "%s\n", zcbor_error_str(err));

	err = cbor_decode_UnorderedMap1(payload_unordered_map1_inv4, sizeof(payload_unordered_map1_inv4), &unordered_map1, NULL);
	zassert_equal(ZCBOR_ERR_ELEMS_NOT_PROCESSED, err, "%s\n", zcbor_error_str(err));
}


ZTEST(cbor_decode_testA, test_unordered_map2)
{
	const uint8_t payload_unordered_map2_1[] = {
		MAP(4),
		1, 0x61, 'a',
		0x63, 'm', 'a', 'p', MAP(3),
			0x20, 0x40,
			0x21, 0x40,
			0xF4, 0x18, 100,
			END
		0x21, 0xF6,
		2, 0x61, 'b',
		END
	};

	const uint8_t payload_unordered_map2_2[] = {
		MAP(2),
		0x63, 'm', 'a', 'p', MAP(1),
			0xF5, 0x39, 0x12, 0x34,
			END
		0x22, 0xF6,
		END
	};

	const uint8_t payload_unordered_map2_3[] = {
		MAP(3),
		2, 0x66, 'f', 'o', 'o', 'b', 'a', 'r',
		0x63, 'm', 'a', 'p', MAP(2),
			0x21, 0x40,
			0x20, 0x40,
			END
		1, 0x60,
		END
	};

	const uint8_t payload_unordered_map2_inv4[] = {
		MAP(4),
		1, 0x61, 'a',
		0x63, 'm', 'a', 'p', MAP(3),
			0x20, 0x40,
			0x21, 0x40,
			0xF4, 0xF6 /* here */,
			END
		0x21, 0xF6,
		2, 0x61, 'b',
		END
	};

	const uint8_t payload_unordered_map2_inv5[] = {
		MAP(4),
		1, 0x61, 'a',
		0x62, 'm', 'a', /* here */ MAP(3),
			0x20, 0x40,
			0x21, 0x40,
			0xF4, 0x18, 100,
			END
		0x21, 0xF6,
		2, 0x61, 'b',
		END
	};

	struct UnorderedMap2 unordered_map2;

	int err = cbor_decode_UnorderedMap2(payload_unordered_map2_1, sizeof(payload_unordered_map2_1), &unordered_map2, NULL);
	zassert_equal(ZCBOR_SUCCESS, err, "%s %d\n", zcbor_error_str(err), err);
	zassert_equal(3, unordered_map2.UnorderedMap2_typeUnion_m_count);
	zassert_equal(typeUnion_type1_m_c, unordered_map2.UnorderedMap2_typeUnion_m[0].typeUnion_choice);
	zassert_equal(typeUnion_type2_m_c, unordered_map2.UnorderedMap2_typeUnion_m[1].typeUnion_choice);
	zassert_equal(typeUnion_typeDefault_m_c, unordered_map2.UnorderedMap2_typeUnion_m[2].typeUnion_choice);
	zassert_equal(1, unordered_map2.UnorderedMap2_typeUnion_m[0].typeUnion_type1_m.type1.len);
	zassert_equal('a', unordered_map2.UnorderedMap2_typeUnion_m[0].typeUnion_type1_m.type1.value[0]);
	zassert_equal(1, unordered_map2.UnorderedMap2_typeUnion_m[1].typeUnion_type2_m.type2.len);
	zassert_equal('b', unordered_map2.UnorderedMap2_typeUnion_m[1].typeUnion_type2_m.type2.value[0]);
	zassert_equal(-2, unordered_map2.UnorderedMap2_typeUnion_m[2].typeUnion_typeDefault_m.typeDefault_key);
	zassert_true(unordered_map2.map_nint1bstr_present);
	zassert_true(unordered_map2.map_nint2bstr_present);
	zassert_true(unordered_map2.map_boolint_present);
	zassert_equal(0, unordered_map2.map_nint1bstr.map_nint1bstr.len);
	zassert_equal(0, unordered_map2.map_nint2bstr.map_nint2bstr.len);
	zassert_false(unordered_map2.map_boolint.UnorderedMap2_map_boolint_key);
	zassert_equal(100, unordered_map2.map_boolint.map_boolint);

	err = cbor_decode_UnorderedMap2(payload_unordered_map2_2, sizeof(payload_unordered_map2_2), &unordered_map2, NULL);
	zassert_equal(ZCBOR_SUCCESS, err, "%s %d\n", zcbor_error_str(err), err);
	zassert_equal(1, unordered_map2.UnorderedMap2_typeUnion_m_count);
	zassert_equal(typeUnion_typeDefault_m_c, unordered_map2.UnorderedMap2_typeUnion_m[0].typeUnion_choice);
	zassert_equal(-3, unordered_map2.UnorderedMap2_typeUnion_m[0].typeUnion_typeDefault_m.typeDefault_key);
	zassert_true(unordered_map2.map_boolint_present);
	zassert_true(unordered_map2.map_boolint.UnorderedMap2_map_boolint_key);
	zassert_equal(-0x1235, unordered_map2.map_boolint.map_boolint);

	err = cbor_decode_UnorderedMap2(payload_unordered_map2_3, sizeof(payload_unordered_map2_3), &unordered_map2, NULL);
	zassert_equal(ZCBOR_SUCCESS, err, "%s %d\n", zcbor_error_str(err), err);
	zassert_equal(2, unordered_map2.UnorderedMap2_typeUnion_m_count);
	zassert_equal(typeUnion_type1_m_c, unordered_map2.UnorderedMap2_typeUnion_m[0].typeUnion_choice);
	zassert_equal(typeUnion_type2_m_c, unordered_map2.UnorderedMap2_typeUnion_m[1].typeUnion_choice);
	zassert_equal(0, unordered_map2.UnorderedMap2_typeUnion_m[0].typeUnion_type1_m.type1.len);
	zassert_equal(6, unordered_map2.UnorderedMap2_typeUnion_m[1].typeUnion_type2_m.type2.len);
	zassert_mem_equal("foobar", unordered_map2.UnorderedMap2_typeUnion_m[1].typeUnion_type2_m.type2.value, 6);
	zassert_true(unordered_map2.map_nint1bstr_present);
	zassert_true(unordered_map2.map_nint2bstr_present);
	zassert_equal(0, unordered_map2.map_nint1bstr.map_nint1bstr.len);
	zassert_equal(0, unordered_map2.map_nint2bstr.map_nint2bstr.len);

	err = cbor_decode_UnorderedMap2(payload_unordered_map2_inv4, sizeof(payload_unordered_map2_inv4), &unordered_map2, NULL);
	zassert_equal(ZCBOR_ERR_ELEMS_NOT_PROCESSED, err, "%s %d\n", zcbor_error_str(err), err);

	err = cbor_decode_UnorderedMap2(payload_unordered_map2_inv5, sizeof(payload_unordered_map2_inv5), &unordered_map2, NULL);
	zassert_equal(ZCBOR_ERR_ELEM_NOT_FOUND, err, "%s %d\n", zcbor_error_str(err), err);
}


ZTEST(cbor_decode_testA, test_unordered_map3)
{
	uint8_t payload_unordered_map3_1[21000];

	ZCBOR_STATE_E(state_e, 1, payload_unordered_map3_1, sizeof(payload_unordered_map3_1), 0);

	zassert_true(zcbor_map_start_encode(state_e, 2000));
	for (uint32_t i = 1; i <= 1000; i++) {
		zassert_true(zcbor_uint32_put(state_e, i), "%d\n", i);
		zassert_true(zcbor_uint32_put(state_e, i), "%d\n", i);
		zassert_true(zcbor_int32_put(state_e, -i), "%d\n", -i);
		zassert_true(zcbor_int32_put(state_e, -i), "%d\n", -i);
	}
	zassert_true(zcbor_map_end_encode(state_e, 2000));

	struct UnorderedMap3 unordered_map3;

	int err = cbor_decode_UnorderedMap3(payload_unordered_map3_1, sizeof(payload_unordered_map3_1), &unordered_map3, NULL);
	zassert_equal(ZCBOR_SUCCESS, err, "%s %d\n", zcbor_error_str(err), err);

	zassert_equal(1000, unordered_map3.UnorderedMap3_uintuint_count, "%d != %d", 1000, unordered_map3.UnorderedMap3_uintuint_count);
	zassert_equal(1000, unordered_map3.UnorderedMap3_nintnint_count, "%d != %d", 1000, unordered_map3.UnorderedMap3_nintnint_count);
}


ZTEST(cbor_decode_testA, test_unordered_map4)
{
	uint8_t payload_unordered_map4_1[50];

	ZCBOR_STATE_E(state_e, 1, payload_unordered_map4_1, sizeof(payload_unordered_map4_1), 0);

	zassert_true(zcbor_map_start_encode(state_e, 6));
	for (int32_t i = -1; i >= -3; i--) {
		zassert_true(zcbor_int32_put(state_e, i), "%d\n", i);
		zassert_true(zcbor_bstr_put_lit(state_e, "world"), NULL);
	}
	for (uint32_t i = 1; i <= 3; i++) {
		zassert_true(zcbor_uint32_put(state_e, i), "%d\n", i);
		zassert_true(zcbor_tstr_put_lit(state_e, "hello"), NULL);
	}
	zassert_true(zcbor_map_end_encode(state_e, 6));

	struct UnorderedMap4 unordered_map4;

	int err = cbor_decode_UnorderedMap4(payload_unordered_map4_1, sizeof(payload_unordered_map4_1), &unordered_map4, NULL);
	zassert_equal(ZCBOR_SUCCESS, err, "%s %d\n", zcbor_error_str(err), err);

	zassert_equal(3, unordered_map4.UnorderedMap4_group1_m_count, "%d != %d", 3, unordered_map4.UnorderedMap4_group1_m_count);
	for (uint32_t i = 1; i <= 3; i++) {
		zassert_equal(i, unordered_map4.UnorderedMap4_group1_m[i - 1].UnorderedMap4_group1_m.group1_uinttstr_key);
		zassert_equal(-i, unordered_map4.UnorderedMap4_group1_m[i - 1].UnorderedMap4_group1_m.group1_nintbstr_key);
	}
}


ZTEST_SUITE(cbor_decode_testA, NULL, NULL, NULL, NULL, NULL);
