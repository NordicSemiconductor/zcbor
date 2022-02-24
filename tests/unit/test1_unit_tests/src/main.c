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

	ZCBOR_STATE_E(state_e, 0, payload, sizeof(payload), 0);
	ZCBOR_STATE_D(state_d, 0, payload, sizeof(payload), 10);

	zassert_true(zcbor_int64_put(state_e, 5), NULL);
	zassert_false(zcbor_int64_expect(state_d, 4), NULL);
	zassert_false(zcbor_int64_expect(state_d, 6), NULL);
	zassert_false(zcbor_int64_expect(state_d, -5), NULL);
	zassert_false(zcbor_int64_expect(state_d, -6), NULL);
	zassert_true(zcbor_int64_expect(state_d, 5), NULL);

	zassert_true(zcbor_int32_put(state_e, 5), NULL);
	zassert_true(zcbor_int64_expect(state_d, 5), NULL);

	zassert_true(zcbor_int64_put(state_e, 5), NULL);
	zassert_true(zcbor_int32_expect(state_d, 5), NULL);

	zassert_true(zcbor_int64_put(state_e, 5000000000), NULL);
	zassert_false(zcbor_int32_decode(state_d, &int32), NULL);
	zassert_true(zcbor_int64_decode(state_d, &int64), NULL);
	zassert_equal(int64, 5000000000, NULL);

	zassert_true(zcbor_int64_encode(state_e, &int64), NULL);
	zassert_false(zcbor_int64_expect(state_d, 5000000001), NULL);
	zassert_true(zcbor_int64_expect(state_d, 5000000000), NULL);

	zassert_true(zcbor_int64_put(state_e, 0x80000000), NULL);
	zassert_false(zcbor_int32_decode(state_d, &int32), NULL);
	zassert_true(zcbor_uint32_expect(state_d, 0x80000000), NULL);

	zassert_true(zcbor_int32_put(state_e, -505), NULL);
	zassert_true(zcbor_int64_expect(state_d, -505), NULL);

	zassert_true(zcbor_int64_put(state_e, -5000000000000), NULL);
	zassert_false(zcbor_int64_expect(state_d, -5000000000001), NULL);
	zassert_false(zcbor_int64_expect(state_d, -4999999999999), NULL);
	zassert_false(zcbor_int64_expect(state_d, 5000000000000), NULL);
	zassert_false(zcbor_int64_expect(state_d, 4999999999999), NULL);
	zassert_true(zcbor_int64_expect(state_d, -5000000000000), NULL);

	zassert_true(zcbor_uint64_put(state_e, ((uint64_t)INT64_MAX + 1)), NULL);
	zassert_false(zcbor_int64_decode(state_d, &int64), NULL);
	zassert_true(zcbor_uint64_expect(state_d, ((uint64_t)INT64_MAX + 1)), NULL);

}


void test_uint64(void)
{
	uint8_t payload[100] = {0};
	uint64_t uint64;
	uint32_t uint32;

	ZCBOR_STATE_E(state_e, 0, payload, sizeof(payload), 0);
	ZCBOR_STATE_D(state_d, 0, payload, sizeof(payload), 10);

	zassert_true(zcbor_uint64_put(state_e, 5), NULL);
	zassert_false(zcbor_uint64_expect(state_d, 4), NULL);
	zassert_false(zcbor_uint64_expect(state_d, 6), NULL);
	zassert_false(zcbor_uint64_expect(state_d, -5), NULL);
	zassert_false(zcbor_uint64_expect(state_d, -6), NULL);
	zassert_true(zcbor_uint64_expect(state_d, 5), NULL);

	zassert_true(zcbor_uint32_put(state_e, 5), NULL);
	zassert_true(zcbor_uint64_expect(state_d, 5), NULL);

	zassert_true(zcbor_uint64_put(state_e, 5), NULL);
	zassert_true(zcbor_uint32_expect(state_d, 5), NULL);

	zassert_true(zcbor_uint64_put(state_e, UINT64_MAX), NULL);
	zassert_false(zcbor_uint32_decode(state_d, &uint32), NULL);
	zassert_true(zcbor_uint64_decode(state_d, &uint64), NULL);
	zassert_equal(uint64, UINT64_MAX, NULL);

	zassert_true(zcbor_uint64_encode(state_e, &uint64), NULL);
	zassert_false(zcbor_uint64_expect(state_d, (UINT64_MAX - 1)), NULL);
	zassert_true(zcbor_uint64_expect(state_d, UINT64_MAX), NULL);
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

	for (int i = 0; i < 1000; i++) {
		large_string[i] = i % 256;
		large_payload[i + 9] = 0;
	}
	for (size_t i = STR_SIZE - 1001; i < STR_SIZE; i++) {
		large_string[i] = i % 256;
		large_payload[i + 9] = 0;
	}

	ZCBOR_STATE_E(state_e, 0, large_payload, PAYL_SIZE, 0);
	ZCBOR_STATE_D(state_d, 0, large_payload, PAYL_SIZE, 10);

	zassert_true(zcbor_tstr_encode(state_e, &tstr), NULL);
	zassert_false(zcbor_bstr_decode(state_d, &tstr_res), NULL);
	zassert_true(zcbor_tstr_decode(state_d, &tstr_res), NULL);
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


void test_stop_on_error(void)
{
	uint8_t payload[100];
	ZCBOR_STATE_E(state_e, 3, payload, sizeof(payload), 0);
	struct zcbor_string failing_string = {.value = NULL, .len = 1000};
	struct zcbor_string dummy_string;
	zcbor_state_t state_backup;
	struct zcbor_state_constant constant_state_backup;

	state_e->constant_state->stop_on_error = true;

	zassert_false(zcbor_tstr_encode(state_e, &failing_string), NULL);
	zassert_equal(ZCBOR_ERR_NO_PAYLOAD, state_e->constant_state->error, "%d\r\n", state_e->constant_state->error);
	memcpy(&state_backup, state_e, sizeof(state_backup));
	memcpy(&constant_state_backup, state_e->constant_state, sizeof(constant_state_backup));

	/* All fail because of ZCBOR_ERR_NO_PAYLOAD */
	zassert_false(zcbor_int32_put(state_e, 1), NULL);
	zassert_false(zcbor_int64_put(state_e, 2), NULL);
	zassert_false(zcbor_uint32_put(state_e, 3), NULL);
	zassert_false(zcbor_uint64_put(state_e, 4), NULL);
	zassert_false(zcbor_int32_encode(state_e, &(int32_t){5}), NULL);
	zassert_false(zcbor_int64_encode(state_e, &(int64_t){6}), NULL);
	zassert_false(zcbor_uint32_encode(state_e, &(uint32_t){7}), NULL);
	zassert_false(zcbor_uint64_encode(state_e, &(uint64_t){8}), NULL);
	zassert_false(zcbor_bstr_put_lit(state_e, "Hello"), NULL);
	zassert_false(zcbor_tstr_put_lit(state_e, "World"), NULL);
	zassert_false(zcbor_tag_encode(state_e, 9), NULL);
	zassert_false(zcbor_bool_put(state_e, true), NULL);
	zassert_false(zcbor_bool_encode(state_e, &(bool){false}), NULL);
	zassert_false(zcbor_float32_put(state_e, 10.5), NULL);
	zassert_false(zcbor_float32_encode(state_e, &(float){11.6}), NULL);
	zassert_false(zcbor_float64_put(state_e, 12.7), NULL);
	zassert_false(zcbor_float64_encode(state_e, &(double){13.8}), NULL);
	zassert_false(zcbor_nil_put(state_e, NULL), NULL);
	zassert_false(zcbor_undefined_put(state_e, NULL), NULL);
	zassert_false(zcbor_bstr_start_encode(state_e), NULL);
	zassert_false(zcbor_bstr_end_encode(state_e), NULL);
	zassert_false(zcbor_list_start_encode(state_e, 1), NULL);
	zassert_false(zcbor_map_start_encode(state_e, 0), NULL);
	zassert_false(zcbor_map_end_encode(state_e, 0), NULL);
	zassert_false(zcbor_list_end_encode(state_e, 1), NULL);
	zassert_false(zcbor_multi_encode(1, (zcbor_encoder_t *)zcbor_int32_put, state_e, (void*)14, 0), NULL);
	zassert_false(zcbor_multi_encode_minmax(1, 1, &(uint_fast32_t){1}, (zcbor_encoder_t *)zcbor_int32_put, state_e, (void*)15, 0), NULL);
	zassert_false(zcbor_present_encode(&(uint_fast32_t){1}, (zcbor_encoder_t *)zcbor_int32_put, state_e, (void*)16), NULL);


	zassert_mem_equal(&state_backup, state_e, sizeof(state_backup), NULL);
	zassert_mem_equal(&constant_state_backup, state_e->constant_state, sizeof(constant_state_backup), NULL);

	zassert_equal(ZCBOR_ERR_NO_PAYLOAD, zcbor_pop_error(state_e), NULL);

	/* All succeed since the error has been popped. */
	zassert_true(zcbor_int32_put(state_e, 1), NULL);
	zassert_true(zcbor_int64_put(state_e, 2), NULL);
	zassert_true(zcbor_uint32_put(state_e, 3), NULL);
	zassert_true(zcbor_uint64_put(state_e, 4), NULL);
	zassert_true(zcbor_int32_encode(state_e, &(int32_t){5}), NULL);
	zassert_true(zcbor_int64_encode(state_e, &(int64_t){6}), NULL);
	zassert_true(zcbor_uint32_encode(state_e, &(uint32_t){7}), NULL);
	zassert_true(zcbor_uint64_encode(state_e, &(uint64_t){8}), NULL);
	zassert_true(zcbor_bstr_put_lit(state_e, "Hello"), NULL);
	zassert_true(zcbor_tstr_put_lit(state_e, "World"), NULL);
	zassert_true(zcbor_tag_encode(state_e, 9), NULL);
	zassert_true(zcbor_tag_encode(state_e, 10), NULL);
	zassert_true(zcbor_bool_put(state_e, true), NULL);
	zassert_true(zcbor_bool_encode(state_e, &(bool){false}), NULL);
	zassert_true(zcbor_float32_put(state_e, 10.5), NULL);
	zassert_true(zcbor_float32_encode(state_e, &(float){11.6}), NULL);
	zassert_true(zcbor_float64_put(state_e, 12.7), NULL);
	zassert_true(zcbor_float64_encode(state_e, &(double){13.8}), NULL);
	zassert_true(zcbor_nil_put(state_e, NULL), NULL);
	zassert_true(zcbor_undefined_put(state_e, NULL), NULL);
	zassert_true(zcbor_bstr_start_encode(state_e), NULL);
	zassert_true(zcbor_bstr_end_encode(state_e), NULL);
	zassert_true(zcbor_list_start_encode(state_e, 1), NULL);
	zassert_true(zcbor_map_start_encode(state_e, 0), NULL);
	zassert_true(zcbor_map_end_encode(state_e, 0), NULL);
	zassert_true(zcbor_list_end_encode(state_e, 1), NULL);
	zassert_true(zcbor_multi_encode(1, (zcbor_encoder_t *)zcbor_int32_put, state_e, (void*)14, 0), NULL);
	zassert_true(zcbor_multi_encode_minmax(1, 1, &(uint_fast32_t){1}, (zcbor_encoder_t *)zcbor_int32_put, state_e, (void*)15, 0), NULL);
	zassert_true(zcbor_present_encode(&(uint_fast32_t){1}, (zcbor_encoder_t *)zcbor_int32_put, state_e, (void*)16), NULL);

	ZCBOR_STATE_D(state_d, 3, payload, sizeof(payload), 30);
	state_d->constant_state->stop_on_error = true;

	zassert_false(zcbor_int32_expect(state_d, 2), NULL);
	zassert_equal(ZCBOR_ERR_WRONG_VALUE, state_d->constant_state->error, "%d\r\n", state_d->constant_state->error);
	memcpy(&state_backup, state_d, sizeof(state_backup));
	memcpy(&constant_state_backup, state_d->constant_state, sizeof(constant_state_backup));

	/* All fail because of ZCBOR_ERR_WRONG_VALUE */
	zassert_false(zcbor_int32_expect(state_d, 1), NULL);
	zassert_false(zcbor_int64_expect(state_d, 2), NULL);
	zassert_false(zcbor_uint32_expect(state_d, 3), NULL);
	zassert_false(zcbor_uint64_expect(state_d, 4), NULL);
	zassert_false(zcbor_int32_decode(state_d, &(int32_t){5}), NULL);
	zassert_false(zcbor_int64_decode(state_d, &(int64_t){6}), NULL);
	zassert_false(zcbor_uint32_decode(state_d, &(uint32_t){7}), NULL);
	zassert_false(zcbor_uint64_decode(state_d, &(uint64_t){8}), NULL);
	zassert_false(zcbor_bstr_expect_lit(state_d, "Hello"), NULL);
	zassert_false(zcbor_tstr_expect_lit(state_d, "World"), NULL);
	zassert_false(zcbor_tag_decode(state_d, &(uint32_t){9}), NULL);
	zassert_false(zcbor_tag_expect(state_d, 10), NULL);
	zassert_false(zcbor_bool_expect(state_d, true), NULL);
	zassert_false(zcbor_bool_decode(state_d, &(bool){false}), NULL);
	zassert_false(zcbor_float32_expect(state_d, 10.5), NULL);
	zassert_false(zcbor_float32_decode(state_d, &(float){11.6}), NULL);
	zassert_false(zcbor_float64_expect(state_d, 12.7), NULL);
	zassert_false(zcbor_float64_decode(state_d, &(double){13.8}), NULL);
	zassert_false(zcbor_nil_expect(state_d, NULL), NULL);
	zassert_false(zcbor_undefined_expect(state_d, NULL), NULL);
	zassert_false(zcbor_bstr_start_decode(state_d, &dummy_string), NULL);
	zassert_false(zcbor_bstr_end_decode(state_d), NULL);
	zassert_false(zcbor_list_start_decode(state_d), NULL);
	zassert_false(zcbor_map_start_decode(state_d), NULL);
	zassert_false(zcbor_map_end_decode(state_d), NULL);
	zassert_false(zcbor_list_end_decode(state_d), NULL);
	zassert_false(zcbor_multi_decode(1, 1, &(uint_fast32_t){1}, (zcbor_decoder_t *)zcbor_int32_expect, state_d, (void*)14, 0), NULL);
	zassert_false(zcbor_int32_expect(state_d, 15), NULL);
	zassert_false(zcbor_present_decode(&(uint_fast32_t){1}, (zcbor_decoder_t *)zcbor_int32_expect, state_d, (void*)16), NULL);

	zassert_mem_equal(&state_backup, state_d, sizeof(state_backup), NULL);
	zassert_mem_equal(&constant_state_backup, state_d->constant_state, sizeof(constant_state_backup), NULL);

	zassert_equal(ZCBOR_ERR_WRONG_VALUE, zcbor_pop_error(state_d), NULL);

	/* All succeed since the error has been popped. */
	zassert_true(zcbor_int32_expect(state_d, 1), NULL);
	zassert_true(zcbor_int64_expect(state_d, 2), NULL);
	zassert_true(zcbor_uint32_expect(state_d, 3), NULL);
	zassert_true(zcbor_uint64_expect(state_d, 4), NULL);
	zassert_true(zcbor_int32_decode(state_d, &(int32_t){5}), NULL);
	zassert_true(zcbor_int64_decode(state_d, &(int64_t){6}), NULL);
	zassert_true(zcbor_uint32_decode(state_d, &(uint32_t){7}), NULL);
	zassert_true(zcbor_uint64_decode(state_d, &(uint64_t){8}), NULL);
	zassert_true(zcbor_bstr_expect_lit(state_d, "Hello"), NULL);
	zassert_true(zcbor_tstr_expect_lit(state_d, "World"), NULL);
	zassert_true(zcbor_tag_decode(state_d, &(uint32_t){9}), NULL);
	zassert_true(zcbor_tag_expect(state_d, 10), NULL);
	zassert_true(zcbor_bool_expect(state_d, true), NULL);
	zassert_true(zcbor_bool_decode(state_d, &(bool){false}), NULL);
	zassert_true(zcbor_float32_expect(state_d, 10.5), NULL);
	zassert_true(zcbor_float32_decode(state_d, &(float){11.6}), NULL);
	zassert_true(zcbor_float64_expect(state_d, 12.7), NULL);
	zassert_true(zcbor_float64_decode(state_d, &(double){13.8}), NULL);
	zassert_true(zcbor_nil_expect(state_d, NULL), NULL);
	zassert_true(zcbor_undefined_expect(state_d, NULL), NULL);
	zassert_true(zcbor_bstr_start_decode(state_d, &dummy_string), NULL);
	zassert_true(zcbor_bstr_end_decode(state_d), NULL);
	zassert_true(zcbor_list_start_decode(state_d), NULL);
	zassert_true(zcbor_map_start_decode(state_d), NULL);
	zassert_true(zcbor_map_end_decode(state_d), NULL);
	zassert_true(zcbor_list_end_decode(state_d), NULL);
	zassert_true(zcbor_multi_decode(1, 1, &(uint_fast32_t){1}, (zcbor_decoder_t *)zcbor_int32_expect, state_d, (void*)14, 0), NULL);
	zassert_true(zcbor_int32_expect(state_d, 15), NULL);
	zassert_true(zcbor_present_decode(&(uint_fast32_t){1}, (zcbor_decoder_t *)zcbor_int32_expect, state_d, (void*)16), NULL);

	/* Everything has been decoded. */
	zassert_equal(state_e->payload, state_d->payload, NULL);
}


void test_main(void)
{
	ztest_test_suite(zcbor_unit_tests,
			 ztest_unit_test(test_int64),
			 ztest_unit_test(test_uint64),
			 ztest_unit_test(test_size64),
			 ztest_unit_test(test_string_macros),
			 ztest_unit_test(test_stop_on_error)
	);
	ztest_run_test_suite(zcbor_unit_tests);
}
