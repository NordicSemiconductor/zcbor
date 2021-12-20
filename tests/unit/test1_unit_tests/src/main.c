/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include "zcbor_decode.h"
#include "zcbor_encode.h"
#include "zcbor_debug.h"

void test_int64(void)
{
	uint8_t payload[100] = {0};
	int64_t int64;
	int32_t int32;
	zcbor_state_t state_d;
	zcbor_state_t state_e;

	zcbor_new_state(&state_e, 1, payload, sizeof(payload), 0);
	zcbor_new_state(&state_d, 1, payload, sizeof(payload), 10);

	zassert_true(zcbor_int64_put(&state_e, 5), NULL);
	zassert_false(zcbor_int64_expect(&state_d, 4), NULL);
	zassert_false(zcbor_int64_expect(&state_d, 6), NULL);
	zassert_false(zcbor_int64_expect(&state_d, -5), NULL);
	zassert_false(zcbor_int64_expect(&state_d, -6), NULL);
	zassert_true(zcbor_int64_expect(&state_d, 5), NULL);

	zassert_true(zcbor_int32_put(&state_e, 5), NULL);
	zassert_true(zcbor_int64_expect(&state_d, 5), NULL);

	zassert_true(zcbor_int64_put(&state_e, 5), NULL);
	zassert_true(zcbor_int32_expect(&state_d, 5), NULL);

	zassert_true(zcbor_int64_put(&state_e, 5000000000), NULL);
	zassert_false(zcbor_int32_decode(&state_d, &int32), NULL);
	zassert_true(zcbor_int64_decode(&state_d, &int64), NULL);
	zassert_equal(int64, 5000000000, NULL);

	zassert_true(zcbor_int64_encode(&state_e, &int64), NULL);
	zassert_false(zcbor_int64_expect(&state_d, 5000000001), NULL);
	zassert_true(zcbor_int64_expect(&state_d, 5000000000), NULL);

	zassert_true(zcbor_int64_put(&state_e, 0x80000000), NULL);
	zassert_false(zcbor_int32_decode(&state_d, &int32), NULL);
	zassert_true(zcbor_uint32_expect(&state_d, 0x80000000), NULL);

	zassert_true(zcbor_int32_put(&state_e, -505), NULL);
	zassert_true(zcbor_int64_expect(&state_d, -505), NULL);

	zassert_true(zcbor_int64_put(&state_e, -5000000000000), NULL);
	zassert_false(zcbor_int64_expect(&state_d, -5000000000001), NULL);
	zassert_false(zcbor_int64_expect(&state_d, -4999999999999), NULL);
	zassert_false(zcbor_int64_expect(&state_d, 5000000000000), NULL);
	zassert_false(zcbor_int64_expect(&state_d, 4999999999999), NULL);
	zassert_true(zcbor_int64_expect(&state_d, -5000000000000), NULL);

	zassert_true(zcbor_uint64_put(&state_e, ((uint64_t)INT64_MAX + 1)), NULL);
	zassert_false(zcbor_int64_decode(&state_d, &int64), NULL);
	zassert_true(zcbor_uint64_expect(&state_d, ((uint64_t)INT64_MAX + 1)), NULL);

}


void test_uint64(void)
{
	uint8_t payload[100] = {0};
	uint64_t uint64;
	uint32_t uint32;
	zcbor_state_t state_d;
	zcbor_state_t state_e;

	zcbor_new_state(&state_e, 1, payload, sizeof(payload), 0);
	zcbor_new_state(&state_d, 1, payload, sizeof(payload), 10);

	zassert_true(zcbor_uint64_put(&state_e, 5), NULL);
	zassert_false(zcbor_uint64_expect(&state_d, 4), NULL);
	zassert_false(zcbor_uint64_expect(&state_d, 6), NULL);
	zassert_false(zcbor_uint64_expect(&state_d, -5), NULL);
	zassert_false(zcbor_uint64_expect(&state_d, -6), NULL);
	zassert_true(zcbor_uint64_expect(&state_d, 5), NULL);

	zassert_true(zcbor_uint32_put(&state_e, 5), NULL);
	zassert_true(zcbor_uint64_expect(&state_d, 5), NULL);

	zassert_true(zcbor_uint64_put(&state_e, 5), NULL);
	zassert_true(zcbor_uint32_expect(&state_d, 5), NULL);

	zassert_true(zcbor_uint64_put(&state_e, UINT64_MAX), NULL);
	zassert_false(zcbor_uint32_decode(&state_d, &uint32), NULL);
	zassert_true(zcbor_uint64_decode(&state_d, &uint64), NULL);
	zassert_equal(uint64, UINT64_MAX, NULL);

	zassert_true(zcbor_uint64_encode(&state_e, &uint64), NULL);
	zassert_false(zcbor_uint64_expect(&state_d, (UINT64_MAX - 1)), NULL);
	zassert_true(zcbor_uint64_expect(&state_d, UINT64_MAX), NULL);
}


#if SIZE_MAX == UINT64_MAX
/* Only runs on 64-bit builds. */
#include <stdlib.h>

#define PAYL_SIZE 0x100000100
#define STR_SIZE 0x100000010

void test_size64(void)
{
	uint8_t *large_payload = malloc(PAYL_SIZE);
	uint8_t *large_string = malloc(STR_SIZE);
	struct zcbor_string tstr = {.value = large_string, .len = STR_SIZE};
	struct zcbor_string tstr_res;
	zcbor_state_t state_d;
	zcbor_state_t state_e;

	for (int i = 0; i < 1000; i++) {
		large_string[i] = i % 256;
		large_payload[i + 9] = 0;
	}
	for (size_t i = STR_SIZE - 1001; i < STR_SIZE; i++) {
		large_string[i] = i % 256;
		large_payload[i + 9] = 0;
	}

	zcbor_new_state(&state_e, 1, large_payload, PAYL_SIZE, 0);
	zcbor_new_state(&state_d, 1, large_payload, PAYL_SIZE, 10);

	zassert_true(zcbor_tstr_encode(&state_e, &tstr), NULL);
	zassert_false(zcbor_bstr_decode(&state_d, &tstr_res), NULL);
	zassert_true(zcbor_tstr_decode(&state_d, &tstr_res), NULL);
	zassert_equal(tstr_res.len, tstr.len, NULL);
	zassert_equal_ptr(tstr_res.value, &large_payload[9], NULL);
	zassert_mem_equal(tstr_res.value, large_string, tstr.len, NULL);
}


#else
void test_size64(void)
{
	printk("Skip on non-64-bit builds.\n");
}
#endif


void test_string_macros(void)
{
	uint8_t payload[100];
	ZCBOR_STATE_E(state_e, 0, payload, sizeof(payload), 0);
	char world[] = {'w', 'o', 'r', 'l', 'd'};

	zassert_true(zcbor_bstr_put_lit(state_e, "Hello"), NULL);
	zassert_true(zcbor_bstr_put_term(state_e, "Hello"), NULL);
	zassert_true(zcbor_bstr_put_arr(state_e, world), NULL);
	zassert_true(zcbor_tstr_put_lit(state_e, "Hello"), NULL);
	zassert_true(zcbor_tstr_put_term(state_e, "Hello"), NULL);
	zassert_true(zcbor_tstr_put_arr(state_e, world), NULL);

	ZCBOR_STATE_D(state_d, 0, payload, sizeof(payload), 6);

	zassert_false(zcbor_bstr_expect_lit(state_d, "Yello"), NULL);
	zassert_false(zcbor_tstr_expect_lit(state_d, "Hello"), NULL);
	zassert_true(zcbor_bstr_expect_lit(state_d, "Hello"), NULL);
	zassert_false(zcbor_bstr_expect_term(state_d, "Hello!"), NULL);
	zassert_true(zcbor_bstr_expect_term(state_d, "Hello"), NULL);
	world[3]++;
	zassert_false(zcbor_bstr_expect_arr(state_d, world), NULL);
	world[3]--;
	zassert_true(zcbor_bstr_expect_arr(state_d, world), NULL);
	zassert_false(zcbor_tstr_expect_lit(state_d, "hello"), NULL);
	zassert_true(zcbor_tstr_expect_lit(state_d, "Hello"), NULL);
	zassert_false(zcbor_tstr_expect_term(state_d, "Helo"), NULL);
	zassert_true(zcbor_tstr_expect_term(state_d, "Hello"), NULL);
	world[2]++;
	zassert_false(zcbor_tstr_expect_arr(state_d, world), NULL);
	world[2]--;
	zassert_false(zcbor_bstr_expect_arr(state_d, world), NULL);
	zassert_true(zcbor_tstr_expect_arr(state_d, world), NULL);
}


void test_main(void)
{
	ztest_test_suite(zcbor_unit_tests,
			 ztest_unit_test(test_int64),
			 ztest_unit_test(test_uint64),
			 ztest_unit_test(test_size64),
			 ztest_unit_test(test_string_macros)
	);
	ztest_run_test_suite(zcbor_unit_tests);
}
