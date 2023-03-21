/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include "senml_encode.h"
#include "zcbor_debug.h" // Enables use of print functions when debugging tests.



#define CONCAT_BYTE(a,b) a ## b

/* LIST() adds a start byte for a list with 'num' elements.
 * MAP() does the same, but for a map.
 * END adds an end byte for the list/map.
 *
 * With ZCBOR_CANONICAL, the start byte contains the list, so no end byte is
 * needed. Without ZCBOR_CANONICAL, the start byte is the same no matter
 * the number of elements, so it needs an explicit end byte.
 */
#ifndef ZCBOR_CANONICAL
#define LIST(num) 0x9F
#define MAP(num) 0xBF
#define END 0xFF,
#else
#define LIST(num) CONCAT_BYTE(0x8, num)
#define MAP(num) CONCAT_BYTE(0xA, num)
#define END
#endif

ZTEST(cbor_encode_test4, test_senml)
{
	struct lwm2m_senml input = {
		._lwm2m_senml__record[0] = {
			._record_bn = {._record_bn = {.value = "Foo", .len = 3}},
			._record_bn_present = 1,
			._record_bt = {._record_bt = 42},
			._record_bt_present = 1,
			._record_n = {._record_n = {.value = "Bar", .len = 3}},
			._record_n_present = 1,
			._record_t = {._record_t = 7},
			._record_t_present = 1,
			._record_union = {
				._union_vb = true,
				._record_union_choice = _union_vb,
			},
			._record_union_present = 1,
			._record__key_value_pair_count = 0,
		},
		._lwm2m_senml__record_count = 1,
	};
	uint8_t payload[100];
	size_t encode_len;
	uint8_t exp_payload1[] = {
		LIST(1), MAP(5), 0x21, 0x63, 'F', 'o', 'o', 0x22, 0x18, 42,
		0x00, 0x63, 'B', 'a', 'r', 0x06, 0x07, 0x04, 0xF5, END END
	};

	zassert_equal(ZCBOR_SUCCESS, cbor_encode_lwm2m_senml(payload, sizeof(payload), &input, &encode_len), NULL);
	zassert_equal(sizeof(exp_payload1), encode_len, NULL);
	zassert_mem_equal(payload, exp_payload1, encode_len, NULL);
}

ZTEST_SUITE(cbor_encode_test4, NULL, NULL, NULL, NULL, NULL);
