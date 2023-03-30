/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include "zcbor_decode.h"
#include "zcbor_encode.h"
#include "zcbor_debug.h"

ZTEST(zcbor_unit_tests, test_int64)
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


ZTEST(zcbor_unit_tests, test_uint64)
{
	uint8_t payload[100] = {0};
	uint64_t uint64;
	uint32_t uint32;

	zcbor_state_t state_e;
	zcbor_new_state(&state_e, 1, payload, sizeof(payload), 0);
	zcbor_state_t state_d;
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

ZTEST(zcbor_unit_tests, test_size)
{
	uint8_t payload[100] = {0};
	size_t read;

	zcbor_state_t state_e;
	zcbor_new_state(&state_e, 1, payload, sizeof(payload), 0);
	zcbor_state_t state_d;
	zcbor_new_state(&state_d, 1, payload, sizeof(payload), 10);

	zassert_true(zcbor_size_put(&state_e, 5), NULL);
	zassert_false(zcbor_size_expect(&state_d, 4), NULL);
	zassert_false(zcbor_size_expect(&state_d, 6), NULL);
	zassert_false(zcbor_size_expect(&state_d, -5), NULL);
	zassert_false(zcbor_size_expect(&state_d, -6), NULL);
	zassert_true(zcbor_size_expect(&state_d, 5), NULL);

	zassert_true(zcbor_int32_put(&state_e, -7), NULL);
	zassert_false(zcbor_size_expect(&state_d, -7), NULL);
	zassert_false(zcbor_size_decode(&state_d, &read), NULL); // Negative number not supported.
	zassert_true(zcbor_int32_expect(&state_d, -7), NULL);

	zassert_true(zcbor_uint32_put(&state_e, 5), NULL);
	zassert_true(zcbor_size_expect(&state_d, 5), NULL);

	zassert_true(zcbor_uint64_put(&state_e, 5), NULL);
	zassert_true(zcbor_size_expect(&state_d, 5), NULL);

	zassert_true(zcbor_uint32_put(&state_e, UINT32_MAX), NULL);
	zassert_true(zcbor_size_decode(&state_d, &read), NULL);
	zassert_equal(read, UINT32_MAX, NULL);

#if SIZE_MAX == UINT64_MAX
	zassert_true(zcbor_uint64_put(&state_e, UINT64_MAX), NULL);
	zassert_true(zcbor_size_decode(&state_d, &read), NULL);
	zassert_equal(read, UINT64_MAX, NULL);
#endif

#if SIZE_MAX == UINT32_MAX
	zassert_true(zcbor_uint64_put(&state_e, UINT64_MAX), NULL);
	zassert_false(zcbor_size_decode(&state_d, &read), NULL);
#endif
}

#if SIZE_MAX == UINT64_MAX
/* Only runs on 64-bit builds. */
#include <stdlib.h>

#define PAYL_SIZE 0x100000100
#define STR_SIZE 0x100000010

ZTEST(zcbor_unit_tests, test_size64)
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
ZTEST(zcbor_unit_tests, test_size64)
{
	printk("Skip on non-64-bit builds.\n");
}
#endif


ZTEST(zcbor_unit_tests, test_string_macros)
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


ZTEST(zcbor_unit_tests, test_stop_on_error)
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
	zassert_false(zcbor_size_put(state_e, 10), NULL);
	zassert_false(zcbor_int32_encode(state_e, &(int32_t){5}), NULL);
	zassert_false(zcbor_int64_encode(state_e, &(int64_t){6}), NULL);
	zassert_false(zcbor_uint32_encode(state_e, &(uint32_t){7}), NULL);
	zassert_false(zcbor_uint64_encode(state_e, &(uint64_t){8}), NULL);
	zassert_false(zcbor_size_encode(state_e, &(size_t){9}), NULL);
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
	zassert_false(zcbor_bstr_end_encode(state_e, NULL), NULL);
	zassert_false(zcbor_list_start_encode(state_e, 1), NULL);
	zassert_false(zcbor_map_start_encode(state_e, 0), NULL);
	zassert_false(zcbor_map_end_encode(state_e, 0), NULL);
	zassert_false(zcbor_list_end_encode(state_e, 1), NULL);
	zassert_false(zcbor_multi_encode(1, (zcbor_encoder_t *)zcbor_int32_put, state_e, (void*)14, 0), NULL);
	zassert_false(zcbor_multi_encode_minmax(1, 1, &(size_t){1}, (zcbor_encoder_t *)zcbor_int32_put, state_e, (void*)15, 0), NULL);
	zassert_false(zcbor_present_encode(&(bool){true}, (zcbor_encoder_t *)zcbor_int32_put, state_e, (void*)16), NULL);


	zassert_mem_equal(&state_backup, state_e, sizeof(state_backup), NULL);
	zassert_mem_equal(&constant_state_backup, state_e->constant_state, sizeof(constant_state_backup), NULL);

	zassert_equal(ZCBOR_ERR_NO_PAYLOAD, zcbor_peek_error(state_e), NULL);
	zassert_equal(ZCBOR_ERR_NO_PAYLOAD, zcbor_pop_error(state_e), NULL);
	zassert_equal(ZCBOR_SUCCESS, zcbor_peek_error(state_e), NULL);

	/* All succeed since the error has been popped. */
	zassert_true(zcbor_int32_put(state_e, 1), NULL);
	zassert_true(zcbor_int64_put(state_e, 2), NULL);
	zassert_true(zcbor_uint32_put(state_e, 3), NULL);
	zassert_true(zcbor_uint64_put(state_e, 4), NULL);
	zassert_true(zcbor_size_put(state_e, 10), NULL);
	zassert_true(zcbor_int32_encode(state_e, &(int32_t){5}), NULL);
	zassert_true(zcbor_int64_encode(state_e, &(int64_t){6}), NULL);
	zassert_true(zcbor_uint32_encode(state_e, &(uint32_t){7}), NULL);
	zassert_true(zcbor_uint64_encode(state_e, &(uint64_t){8}), NULL);
	zassert_true(zcbor_size_encode(state_e, &(size_t){9}), NULL);
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
	zassert_true(zcbor_bstr_end_encode(state_e, NULL), NULL);
	zassert_true(zcbor_list_start_encode(state_e, 1), NULL);
	zassert_true(zcbor_map_start_encode(state_e, 0), NULL);
	zassert_true(zcbor_map_end_encode(state_e, 0), NULL);
	zassert_true(zcbor_list_end_encode(state_e, 1), NULL);
	zassert_true(zcbor_multi_encode(1, (zcbor_encoder_t *)zcbor_int32_put, state_e, (void*)14, 0), NULL);
	zassert_true(zcbor_multi_encode_minmax(1, 1, &(size_t){1}, (zcbor_encoder_t *)zcbor_int32_put, state_e, (void*)15, 0), NULL);
	zassert_true(zcbor_present_encode(&(bool){true}, (zcbor_encoder_t *)zcbor_int32_put, state_e, (void*)16), NULL);

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
	zassert_false(zcbor_size_expect(state_d, 10), NULL);
	zassert_false(zcbor_int32_decode(state_d, &(int32_t){5}), NULL);
	zassert_false(zcbor_int64_decode(state_d, &(int64_t){6}), NULL);
	zassert_false(zcbor_uint32_decode(state_d, &(uint32_t){7}), NULL);
	zassert_false(zcbor_uint64_decode(state_d, &(uint64_t){8}), NULL);
	zassert_false(zcbor_size_decode(state_d, &(size_t){9}), NULL);
	zassert_false(zcbor_bstr_expect_lit(state_d, "Hello"), NULL);
	zassert_false(zcbor_tstr_expect_lit(state_d, "World"), NULL);
	zassert_false(zcbor_tag_decode(state_d, &(uint32_t){9}), NULL);
	zassert_false(zcbor_tag_expect(state_d, 10), NULL);
	zassert_false(zcbor_bool_expect(state_d, true), NULL);
	zassert_false(zcbor_bool_decode(state_d, &(bool){false}), NULL);
	zassert_false(zcbor_float32_expect(state_d, 10.5), NULL);
	zassert_false(zcbor_float32_decode(state_d, &(float){11.6}), NULL);
#ifndef CONFIG_BOARD_QEMU_MALTA_BE
	zassert_false(zcbor_float64_expect(state_d, 12.7), NULL);
#endif
	zassert_false(zcbor_float64_decode(state_d, &(double){13.8}), NULL);
	zassert_false(zcbor_nil_expect(state_d, NULL), NULL);
	zassert_false(zcbor_undefined_expect(state_d, NULL), NULL);
	zassert_false(zcbor_bstr_start_decode(state_d, &dummy_string), NULL);
	zassert_false(zcbor_bstr_end_decode(state_d), NULL);
	zassert_false(zcbor_list_start_decode(state_d), NULL);
	zassert_false(zcbor_map_start_decode(state_d), NULL);
	zassert_false(zcbor_map_end_decode(state_d), NULL);
	zassert_false(zcbor_list_end_decode(state_d), NULL);
	zassert_false(zcbor_multi_decode(1, 1, &(size_t){1}, (zcbor_decoder_t *)zcbor_int32_expect, state_d, (void*)14, 0), NULL);
	zassert_false(zcbor_int32_expect(state_d, 15), NULL);
	zassert_false(zcbor_present_decode(&(bool){true}, (zcbor_decoder_t *)zcbor_int32_expect, state_d, (void*)16), NULL);

	zassert_mem_equal(&state_backup, state_d, sizeof(state_backup), NULL);
	zassert_mem_equal(&constant_state_backup, state_d->constant_state, sizeof(constant_state_backup), NULL);

	zassert_equal(ZCBOR_ERR_WRONG_VALUE, zcbor_pop_error(state_d), NULL);

	/* All succeed since the error has been popped. */
	zassert_true(zcbor_int32_expect(state_d, 1), NULL);
	zassert_true(zcbor_int64_expect(state_d, 2), NULL);
	zassert_true(zcbor_uint32_expect(state_d, 3), NULL);
	zassert_true(zcbor_uint64_expect(state_d, 4), NULL);
	zassert_true(zcbor_size_expect(state_d, 10), NULL);
	zassert_true(zcbor_int32_decode(state_d, &(int32_t){5}), NULL);
	zassert_true(zcbor_int64_decode(state_d, &(int64_t){6}), NULL);
	zassert_true(zcbor_uint32_decode(state_d, &(uint32_t){7}), NULL);
	zassert_true(zcbor_uint64_decode(state_d, &(uint64_t){8}), NULL);
	zassert_true(zcbor_size_decode(state_d, &(size_t){9}), NULL);
	zassert_true(zcbor_bstr_expect_lit(state_d, "Hello"), NULL);
	zassert_true(zcbor_tstr_expect_lit(state_d, "World"), NULL);
	zassert_true(zcbor_tag_decode(state_d, &(uint32_t){9}), NULL);
	zassert_true(zcbor_tag_expect(state_d, 10), NULL);
	zassert_true(zcbor_bool_expect(state_d, true), NULL);
	zassert_true(zcbor_bool_decode(state_d, &(bool){false}), NULL);
	zassert_true(zcbor_float32_expect(state_d, 10.5), NULL);
	zassert_true(zcbor_float32_decode(state_d, &(float){11.6}), NULL);
#ifdef CONFIG_BOARD_QEMU_MALTA_BE
	zassert_true(zcbor_float64_decode(state_d, &(double){12.7}), NULL);
#else
	zassert_true(zcbor_float64_expect(state_d, 12.7), NULL);
#endif
	zassert_true(zcbor_float64_decode(state_d, &(double){13.8}), NULL);
	zassert_true(zcbor_nil_expect(state_d, NULL), NULL);
	zassert_true(zcbor_undefined_expect(state_d, NULL), NULL);
	zassert_true(zcbor_bstr_start_decode(state_d, &dummy_string), NULL);
	zassert_true(zcbor_bstr_end_decode(state_d), NULL);
	zassert_true(zcbor_list_start_decode(state_d), NULL);
	zassert_true(zcbor_map_start_decode(state_d), NULL);
	zassert_true(zcbor_map_end_decode(state_d), NULL);
	zassert_true(zcbor_list_end_decode(state_d), NULL);
	zassert_true(zcbor_multi_decode(1, 1, &(size_t){1}, (zcbor_decoder_t *)zcbor_int32_expect, state_d, (void*)14, 0), NULL);
	zassert_true(zcbor_int32_expect(state_d, 15), NULL);
	zassert_true(zcbor_present_decode(&(bool){1}, (zcbor_decoder_t *)zcbor_int32_expect, state_d, (void*)16), NULL);

	/* Everything has been decoded. */
	zassert_equal(state_e->payload, state_d->payload, NULL);
}


/** The string "HelloWorld" is split into 2 fragments, and a number of different
 * functions are called to test that they respond correctly to the situation.
**/
ZTEST(zcbor_unit_tests, test_fragments)
{
	uint8_t payload[100];
	ZCBOR_STATE_E(state_e, 0, payload, sizeof(payload), 0);
	struct zcbor_string output;
	struct zcbor_string_fragment output_frags[2];

	zassert_true(zcbor_bstr_put_lit(state_e, "HelloWorld"), NULL);

	ZCBOR_STATE_D(state_d, 0, payload, 8, 1);
	ZCBOR_STATE_D(state_d2, 0, payload, sizeof(payload), 1);

	zassert_true(zcbor_bstr_decode(state_d2, &output), NULL);
	zassert_false(zcbor_payload_at_end(state_d2), NULL);
	zassert_false(zcbor_bstr_decode(state_d, &output), NULL);
	zassert_false(zcbor_payload_at_end(state_d), NULL);
	zassert_true(zcbor_bstr_decode_fragment(state_d, &output_frags[0]), NULL);
	zassert_equal_ptr(&payload[1], output_frags[0].fragment.value, NULL);
	zassert_equal(7, output_frags[0].fragment.len, NULL);
	zassert_equal(10, output_frags[0].total_len, "%d != %d\r\n", 10, output_frags[0].total_len);
	zassert_equal(0, output_frags[0].offset, NULL);
	zassert_false(zcbor_is_last_fragment(&output_frags[0]), NULL);

	zassert_true(zcbor_payload_at_end(state_d), NULL);
	zcbor_update_state(state_d, &payload[8], sizeof(payload) - 8);
	zassert_false(zcbor_bstr_decode_fragment(state_d, &output_frags[1]), NULL);
	zcbor_next_fragment(state_d, &output_frags[0], &output_frags[1]);
	zassert_equal_ptr(&payload[8], output_frags[1].fragment.value, NULL);
	zassert_equal(3, output_frags[1].fragment.len, "%d != %d\r\n", 3, output_frags[1].fragment.len);
	zassert_equal(10, output_frags[1].total_len, NULL);
	zassert_equal(7, output_frags[1].offset, NULL);
	zassert_true(zcbor_is_last_fragment(&output_frags[1]), NULL);

	uint8_t spliced[11];
	output.value = spliced;
	output.len = sizeof(spliced);

	zassert_true(zcbor_validate_string_fragments(output_frags, 2), NULL);
	zassert_true(zcbor_splice_string_fragments(output_frags, 2, spliced, &output.len), NULL);

	zassert_equal(10, output.len, NULL);
	zassert_mem_equal(output.value, "HelloWorld", 10, NULL);
}


/** The long string "HelloWorld1HelloWorld2..." is split into 18 fragments.
 *
 * First, zcbor_validate_string_fragments() is checked to be true, then various
 * errors are introduced one by one and zcbor_validate_string_fragments() is
 * checked to be false for all of them in turn.
 */
ZTEST(zcbor_unit_tests, test_validate_fragments)
{
	uint8_t payload[200];
	uint8_t frag_payload[15];

	ZCBOR_STATE_E(state_e, 0, payload, sizeof(payload), 0);
	struct zcbor_string output;
	struct zcbor_string_fragment output_frags[18];
	struct zcbor_string_fragment output_frags_backup;

	// Start positive test

	zassert_true(zcbor_bstr_put_lit(state_e,
		"HelloWorld1HelloWorld2HelloWorld3HelloWorld4HelloWorld5HelloWorld6" \
		"HelloWorld7HelloWorld8HelloWorld9HelloWorldAHelloWorldBHelloWorldC" \
		"HelloWorldDHelloWorldEHelloWorldFHelloWorldGHelloWorldHHelloWorldI"),
		NULL);

	ZCBOR_STATE_D(state_d, 0, payload, 13, 1);
	ZCBOR_STATE_D(state_d2, 0, payload, sizeof(payload), 1);

	zassert_true(zcbor_bstr_decode(state_d2, &output), NULL);
	zassert_true(zcbor_bstr_decode_fragment(state_d, &output_frags[0]), NULL);

	for (int i = 1; i < 18; i++) {
		zassert_true(zcbor_payload_at_end(state_d), NULL);
		zassert_false(zcbor_is_last_fragment(&output_frags[i - 1]), NULL);
		memcpy(frag_payload, &payload[11 * i + 2], 11); // + 2 because of the CBOR header
		zcbor_update_state(state_d, frag_payload, 11);
		zcbor_next_fragment(state_d, &output_frags[i - 1], &output_frags[i]);
	}
	zassert_true(zcbor_payload_at_end(state_d), NULL);
	zassert_true(zcbor_is_last_fragment(&output_frags[17]), NULL);

	uint8_t spliced[200];
	size_t out_len = sizeof(spliced);

	zassert_true(zcbor_validate_string_fragments(output_frags, 18), NULL);
	zassert_true(zcbor_splice_string_fragments(output_frags, 18, spliced,
			&out_len), NULL);

	zassert_equal(198, output.len, NULL);
	zassert_mem_equal(output.value,
		"HelloWorld1HelloWorld2HelloWorld3HelloWorld4HelloWorld5HelloWorld6" \
		"HelloWorld7HelloWorld8HelloWorld9HelloWorldAHelloWorldBHelloWorldC" \
		"HelloWorldDHelloWorldEHelloWorldFHelloWorldGHelloWorldHHelloWorldI",
		198, NULL);

	// Start negative tests

	zassert_false(zcbor_validate_string_fragments(output_frags, 17), NULL); // Last fragment missing

	output_frags[2].total_len--;
	zassert_false(zcbor_validate_string_fragments(output_frags, 18), NULL); // Mismatch total_len
	output_frags[2].total_len += 2;
	zassert_false(zcbor_validate_string_fragments(output_frags, 18), NULL); // Mismatch total_len
	output_frags[2].total_len--;

	memcpy(&output_frags_backup, &output_frags[16], sizeof(output_frags[0]));
	memcpy(&output_frags[16], &output_frags[17], sizeof(output_frags[0])); // Move last fragment

	zassert_false(zcbor_validate_string_fragments(output_frags, 17), NULL); // Second-to-last fragment missing
	memcpy(&output_frags[16], &output_frags_backup, sizeof(output_frags[0])); // Restore

	output_frags[6].offset++;
	output_frags[7].offset--;
	zassert_false(zcbor_validate_string_fragments(output_frags, 18), NULL); // Slight error in offset vs. fragment lengths.
	output_frags[6].offset--;
	output_frags[7].offset++;

	for (int i = 0; i < 18; i++) {
		output_frags[i].total_len -= output_frags[17].fragment.len;
	}

	zassert_true(zcbor_validate_string_fragments(output_frags, 17), NULL); // Should work with 17 fragments now.
	zassert_false(zcbor_validate_string_fragments(output_frags, 18), NULL); // Last fragment will extend past total_len.

	for (int i = 0; i < 18; i++) {
		output_frags[i].total_len += (output_frags[17].fragment.len - 1);
	}
	zassert_false(zcbor_validate_string_fragments(output_frags, 18), NULL); // Last fragment will extend past total_len by a single byte.
	output_frags[17].fragment.len--;
	zassert_true(zcbor_validate_string_fragments(output_frags, 18), NULL); // Should work with shorter last fragment
	output_frags[17].fragment.len++; // Restore
	for (int i = 0; i < 18; i++) {
		output_frags[i].total_len += 1; // Restore
	}

	zassert_true(zcbor_validate_string_fragments(output_frags, 18), NULL); // Check that all errors were restored correctly.
}


/** This test creates the following structure, wrapped in a BSTR:
 *
 *  (
 *      42,
 *      "Hello World",
 *      [
 *          true,
 *          nil,
 *      ]
 *  )
 *
 *  This structures is then split in three fragments (output_frags) like so:
 *  1st fragment (8 bytes):
 *  (
 *      42,
 *      "Hell
 *
 *  2nd fragment (8 bytes):
 *           o World",
 *      [
 *
 *  3rd fragment (3 bytes or 2 bytes if ZCBOR_CANONICAL is defined):
 *          true,
 *          nil,
 *      ]
 *  )
 *
 *  This means that the TSTR has to be handled using fragments (tstr_frags)
 *  as well.
 */
ZTEST(zcbor_unit_tests, test_bstr_cbor_fragments)
{
	uint8_t payload[100];
	ZCBOR_STATE_E(state_e, 2, payload, sizeof(payload), 0);
	struct zcbor_string output;
	struct zcbor_string_fragment output_frags[3];
	struct zcbor_string_fragment tstr_frags[2];

	zassert_true(zcbor_bstr_start_encode(state_e), NULL); // 1 B
	zassert_true(zcbor_uint32_put(state_e, 42), NULL); // 2 B
	zassert_true(zcbor_tstr_put_lit(state_e, "Hello World"), NULL); // 12 B
	zassert_true(zcbor_list_start_encode(state_e, 2), NULL); // 1 B
	zassert_true(zcbor_bool_put(state_e, true), NULL); // 1 B
	zassert_true(zcbor_nil_put(state_e, NULL), NULL); // 1 B
	zassert_true(zcbor_list_end_encode(state_e, 2), NULL); // 1 B
	zassert_true(zcbor_bstr_end_encode(state_e, NULL), NULL); // 0 B

	ZCBOR_STATE_D(state_d, 2, payload, 8, 1);
	ZCBOR_STATE_D(state_d2, 0, payload, sizeof(payload), 1);

#ifdef ZCBOR_CANONICAL
	#define EXP_TOTAL_LEN 17
#else
	#define EXP_TOTAL_LEN 18
#endif

	zassert_true(zcbor_bstr_decode(state_d2, &output), NULL);
	zassert_false(zcbor_bstr_start_decode(state_d, &output), NULL);
	zassert_true(zcbor_bstr_start_decode_fragment(state_d, &output_frags[0]), NULL);
	zassert_equal_ptr(&payload[1], output_frags[0].fragment.value, NULL);
	zassert_equal(7, output_frags[0].fragment.len, NULL);
	zassert_equal(EXP_TOTAL_LEN, output_frags[0].total_len, "%d != %d\r\n", EXP_TOTAL_LEN, output_frags[0].total_len);
	zassert_equal(0, output_frags[0].offset, NULL);
	zassert_false(zcbor_is_last_fragment(&output_frags[0]), NULL);
	zassert_true(zcbor_uint32_expect(state_d, 42), NULL);
	zassert_false(zcbor_tstr_expect_lit(state_d, "Hello World"), NULL);
	zassert_true(zcbor_tstr_decode_fragment(state_d, &tstr_frags[0]), NULL);
	zassert_equal_ptr(&payload[4], tstr_frags[0].fragment.value, NULL);
	zassert_equal(4, tstr_frags[0].fragment.len, NULL);
	zassert_equal(11, tstr_frags[0].total_len, NULL);
	zassert_equal(0, tstr_frags[0].offset, NULL);

	zassert_true(zcbor_payload_at_end(state_d), NULL);
	zcbor_update_state(state_d, &payload[8], 8);
	zassert_false(zcbor_bstr_decode_fragment(state_d, &output_frags[1]), NULL);
	zcbor_bstr_next_fragment(state_d, &output_frags[0], &output_frags[1]);
	zassert_equal_ptr(&payload[8], output_frags[1].fragment.value, NULL);
	zassert_equal(8, output_frags[1].fragment.len, "%d != %d\r\n", 3, output_frags[1].fragment.len);
	zassert_equal(EXP_TOTAL_LEN, output_frags[1].total_len, "%d != %d\r\n", EXP_TOTAL_LEN, output_frags[1].total_len);
	zassert_equal(7, output_frags[1].offset, NULL);
	zassert_false(zcbor_is_last_fragment(&output_frags[1]), NULL);
	zcbor_next_fragment(state_d, &tstr_frags[0], &tstr_frags[1]);
	zassert_equal_ptr(&payload[8], tstr_frags[1].fragment.value, NULL);
	zassert_equal(7, tstr_frags[1].fragment.len, "%d != %d\r\n", 7, tstr_frags[1].fragment.len);
	zassert_equal(11, tstr_frags[1].total_len, NULL);
	zassert_equal(4, tstr_frags[1].offset, NULL);
	zassert_true(zcbor_is_last_fragment(&tstr_frags[1]), NULL);
	zassert_true(zcbor_list_start_decode(state_d), NULL);

	zassert_true(zcbor_payload_at_end(state_d), NULL);
	zcbor_update_state(state_d, &payload[16], sizeof(payload) - 16);
	zassert_false(zcbor_bstr_decode_fragment(state_d, &output_frags[2]), NULL);
	zcbor_bstr_next_fragment(state_d, &output_frags[1], &output_frags[2]);
	zassert_equal_ptr(&payload[16], output_frags[2].fragment.value, NULL);
	zassert_equal(EXP_TOTAL_LEN - 15,
			output_frags[2].fragment.len, NULL);
	zassert_equal(EXP_TOTAL_LEN, output_frags[2].total_len, NULL);
	zassert_equal(15, output_frags[2].offset, NULL);
	zassert_true(zcbor_is_last_fragment(&output_frags[2]), NULL);
	zassert_true(zcbor_bool_expect(state_d, true), NULL);
	zassert_true(zcbor_nil_expect(state_d, NULL), NULL);
	zassert_true(zcbor_list_end_decode(state_d), NULL);

	uint8_t spliced[19];
	output.value = spliced;
	output.len = sizeof(spliced);

	zassert_true(zcbor_validate_string_fragments(output_frags, 3), NULL);
	zassert_true(zcbor_splice_string_fragments(output_frags, 3, spliced, &output.len), NULL);

	zassert_equal(EXP_TOTAL_LEN, output.len, NULL);
	zassert_mem_equal(output.value, &payload[1], EXP_TOTAL_LEN, NULL);

	output.len = sizeof(spliced);

	zassert_true(zcbor_validate_string_fragments(tstr_frags, 2), NULL);
	zassert_true(zcbor_splice_string_fragments(tstr_frags, 2, spliced, &output.len), NULL);

	zassert_equal(11, output.len, NULL);
	zassert_mem_equal(output.value, &payload[4], 11, NULL);
}


ZTEST(zcbor_unit_tests, test_canonical_list)
{
#ifndef ZCBOR_CANONICAL
	printk("Skip on non-canonical builds.\n");
#else
	uint8_t payload1[100];
	uint8_t payload2[100];
	uint8_t exp_payload[] = {0x8A, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	ZCBOR_STATE_E(state_e1, 1, payload1, sizeof(payload1), 0);
	ZCBOR_STATE_E(state_e2, 1, payload2, sizeof(payload2), 0);

	zassert_true(zcbor_list_start_encode(state_e1, 10), "%d\r\n", state_e1->constant_state->error);
	for (int i = 0; i < 30; i++) {
		zassert_true(zcbor_uint32_put(state_e1, i), NULL);
	}
	zassert_false(zcbor_list_end_encode(state_e1, 10), NULL);
	zassert_equal(ZCBOR_ERR_HIGH_ELEM_COUNT, zcbor_pop_error(state_e1), NULL);

	zassert_true(zcbor_list_start_encode(state_e2, 1000), NULL);
	for (int i = 0; i < 10; i++) {
		zassert_true(zcbor_uint32_put(state_e2, i), NULL);
	}
	zassert_true(zcbor_list_end_encode(state_e2, 1000), NULL);
	zassert_equal(sizeof(exp_payload), state_e2->payload - payload2, NULL);
	zassert_mem_equal(exp_payload, payload2, sizeof(exp_payload), NULL);
#endif
}


ZTEST(zcbor_unit_tests, test_int)
{
	uint8_t payload1[100];
	ZCBOR_STATE_E(state_e, 1, payload1, sizeof(payload1), 0);
	ZCBOR_STATE_D(state_d, 1, payload1, sizeof(payload1), 16);

	/* Encode all numbers */
	/* Arbitrary positive numbers in each size */
	int8_t int8 = 12;
	int16_t int16 = 1234;
	int32_t int32 = 12345678;
	int64_t int64 = 1234567812345678;

	zassert_true(zcbor_int_encode(state_e, &int8, sizeof(int8)), NULL);
	zassert_true(zcbor_int_encode(state_e, &int16, sizeof(int16)), NULL);
	zassert_true(zcbor_int_encode(state_e, &int32, sizeof(int32)), NULL);
	zassert_true(zcbor_int_encode(state_e, &int64, sizeof(int64)), NULL);

	/* Arbitrary negative numbers in each size */
	int8 = -12;
	int16 = -1234;
	int32 = -12345678;
	int64 = -1234567812345678;

	zassert_true(zcbor_int_encode(state_e, &int8, sizeof(int8)), NULL);
	zassert_true(zcbor_int_encode(state_e, &int16, sizeof(int16)), NULL);
	zassert_true(zcbor_int_encode(state_e, &int32, sizeof(int32)), NULL);
	zassert_true(zcbor_int_encode(state_e, &int64, sizeof(int64)), NULL);

	/* Check against overflow (negative). */
	zassert_true(zcbor_int32_put(state_e, INT8_MIN - 1), NULL);
	zassert_true(zcbor_int32_put(state_e, INT16_MIN - 1), NULL);
	zassert_true(zcbor_int64_put(state_e, (int64_t)INT32_MIN - 1), NULL);
	/* Check absolute minimum number. */
	zassert_true(zcbor_int64_put(state_e, INT64_MIN), NULL);

	/* Check against overflow (positive). */
	zassert_true(zcbor_int32_put(state_e, INT8_MAX + 1), NULL);
	zassert_true(zcbor_int32_put(state_e, INT16_MAX + 1), NULL);
	zassert_true(zcbor_int64_put(state_e, (int64_t)INT32_MAX + 1), NULL);
	/* Check absolute maximum number. */
	zassert_true(zcbor_int64_put(state_e, INT64_MAX), NULL);

	/* Decode all numbers */
	/* Arbitrary positive numbers in each size */
	zassert_true(zcbor_int_decode(state_d, &int8, sizeof(int8)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int16, sizeof(int8)), NULL);
	zassert_true(zcbor_int_decode(state_d, &int16, sizeof(int16)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int32, sizeof(int8)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int32, sizeof(int16)), NULL);
	zassert_true(zcbor_int_decode(state_d, &int32, sizeof(int32)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int64, sizeof(int8)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int64, sizeof(int16)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int64, sizeof(int32)), NULL);
	zassert_true(zcbor_int_decode(state_d, &int64, sizeof(int64)), NULL);

	zassert_equal(int8, 12, "%d\r\n", int8);
	zassert_equal(int16, 1234, "%d\r\n", int16);
	zassert_equal(int32, 12345678, "%d\r\n", int32);
	zassert_equal(int64, 1234567812345678, "%d\r\n", int64);

	/* Arbitrary negative numbers in each size */
	zassert_true(zcbor_int_decode(state_d, &int8, sizeof(int8)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int16, sizeof(int8)), NULL);
	zassert_true(zcbor_int_decode(state_d, &int16, sizeof(int16)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int32, sizeof(int8)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int32, sizeof(int16)), NULL);
	zassert_true(zcbor_int_decode(state_d, &int32, sizeof(int32)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int64, sizeof(int8)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int64, sizeof(int16)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int64, sizeof(int32)), NULL);
	zassert_true(zcbor_int_decode(state_d, &int64, sizeof(int64)), NULL);

	zassert_equal(int8, -12, NULL);
	zassert_equal(int16, -1234, NULL);
	zassert_equal(int32, -12345678, NULL);
	zassert_equal(int64, -1234567812345678, NULL);

	/* Check against overflow (negative). */
	zassert_false(zcbor_int_decode(state_d, &int16, sizeof(int8)), NULL);
	zassert_true(zcbor_int_decode(state_d, &int16, sizeof(int16)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int32, sizeof(int8)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int32, sizeof(int16)), NULL);
	zassert_true(zcbor_int_decode(state_d, &int32, sizeof(int32)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int64, sizeof(int8)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int64, sizeof(int16)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int64, sizeof(int32)), NULL);
	zassert_true(zcbor_int_decode(state_d, &int64, sizeof(int64)), NULL);

	zassert_equal(int16, INT8_MIN - 1, NULL);
	zassert_equal(int32, INT16_MIN - 1, NULL);
	zassert_equal(int64, (int64_t)INT32_MIN - 1, NULL);

	/* Check absolute minimum number. */
	zassert_true(zcbor_int_decode(state_d, &int64, sizeof(int64)), NULL);

	zassert_equal(int64, INT64_MIN, NULL);

	/* Check against overflow (positive). */
	zassert_false(zcbor_int_decode(state_d, &int16, sizeof(int8)), NULL);
	zassert_true(zcbor_int_decode(state_d, &int16, sizeof(int16)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int32, sizeof(int8)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int32, sizeof(int16)), NULL);
	zassert_true(zcbor_int_decode(state_d, &int32, sizeof(int32)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int64, sizeof(int8)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int64, sizeof(int16)), NULL);
	zassert_false(zcbor_int_decode(state_d, &int64, sizeof(int32)), NULL);
	zassert_true(zcbor_int_decode(state_d, &int64, sizeof(int64)), NULL);

	zassert_equal(int16, INT8_MAX + 1, NULL);
	zassert_equal(int32, INT16_MAX + 1, NULL);
	zassert_equal(int64, (int64_t)INT32_MAX + 1, NULL);

	/* Check absolute maximum number. */
	zassert_true(zcbor_int_decode(state_d, &int64, sizeof(int64)), NULL);

	zassert_equal(int64, INT64_MAX, NULL);
}


ZTEST(zcbor_unit_tests, test_uint)
{
	uint8_t payload1[100];
	ZCBOR_STATE_E(state_e, 1, payload1, sizeof(payload1), 0);
	ZCBOR_STATE_D(state_d, 1, payload1, sizeof(payload1), 16);

	/* Encode all numbers */
	/* Arbitrary positive numbers in each size */
	uint8_t uint8 = 12;
	uint16_t uint16 = 1234;
	uint32_t uint32 = 12345678;
	uint64_t uint64 = 1234567812345678;

	zassert_true(zcbor_uint_encode(state_e, &uint8, sizeof(uint8)), NULL);
	zassert_true(zcbor_uint_encode(state_e, &uint16, sizeof(uint16)), NULL);
	zassert_true(zcbor_uint_encode(state_e, &uint32, sizeof(uint32)), NULL);
	zassert_true(zcbor_uint_encode(state_e, &uint64, sizeof(uint64)), NULL);

	/* Check absolute maximum number. */
	zassert_true(zcbor_uint64_put(state_e, UINT64_MAX), NULL);

	/* Decode all numbers */
	/* Arbitrary positive numbers in each size */
	zassert_true(zcbor_uint_decode(state_d, &uint8, sizeof(uint8)), NULL);
	zassert_false(zcbor_uint_decode(state_d, &uint16, sizeof(uint8)), NULL);
	zassert_true(zcbor_uint_decode(state_d, &uint16, sizeof(uint16)), NULL);
	zassert_false(zcbor_uint_decode(state_d, &uint32, sizeof(uint8)), NULL);
	zassert_false(zcbor_uint_decode(state_d, &uint32, sizeof(uint16)), NULL);
	zassert_true(zcbor_uint_decode(state_d, &uint32, sizeof(uint32)), NULL);
	zassert_false(zcbor_uint_decode(state_d, &uint64, sizeof(uint8)), NULL);
	zassert_false(zcbor_uint_decode(state_d, &uint64, sizeof(uint16)), NULL);
	zassert_false(zcbor_uint_decode(state_d, &uint64, sizeof(uint32)), NULL);
	zassert_true(zcbor_uint_decode(state_d, &uint64, sizeof(uint64)), NULL);

	zassert_equal(uint8, 12, "%d\r\n", uint8);
	zassert_equal(uint16, 1234, "%d\r\n", uint16);
	zassert_equal(uint32, 12345678, "%d\r\n", uint32);
	zassert_equal(uint64, 1234567812345678, "%d\r\n", uint64);

	/* Check absolute maximum number. */
	zassert_true(zcbor_uint_decode(state_d, &uint64, sizeof(uint64)), NULL);

	zassert_equal(uint64, UINT64_MAX, NULL);
}


/** This tests a regression in big-endian encoding, where small numbers (like 0)
  * where handled incorrectly (1-off), because of an error in get_result().
  */
ZTEST(zcbor_unit_tests, test_encode_int_0)
{
	uint8_t payload1[100];
	uint32_t values_32[2] = {0, UINT32_MAX};
	uint64_t values_64[2] = {0, UINT64_MAX};
	ZCBOR_STATE_E(state_e, 1, payload1, sizeof(payload1), 0);
	ZCBOR_STATE_D(state_d, 1, payload1, sizeof(payload1), 16);

	zassert_true(zcbor_uint32_put(state_e, values_32[0]));
	zassert_true(zcbor_uint64_put(state_e, values_64[0]));
	zassert_true(zcbor_uint32_expect(state_d, 0));
	zassert_true(zcbor_uint64_expect(state_d, 0));
}


ZTEST(zcbor_unit_tests, test_simple)
{
	uint8_t payload1[100];
	ZCBOR_STATE_E(state_e, 1, payload1, sizeof(payload1), 0);
	ZCBOR_STATE_D(state_d, 1, payload1, sizeof(payload1), 16);
	uint8_t simple1 = 0;
	uint8_t simple2 = 2;

	zassert_true(zcbor_simple_encode(state_e, &simple1), NULL);
	zassert_true(zcbor_simple_put(state_e, 14), NULL);
	zassert_true(zcbor_simple_put(state_e, 22), NULL);
	zassert_true(zcbor_simple_put(state_e, 24), NULL);
	zassert_true(zcbor_simple_put(state_e, 255), NULL);

	zassert_true(zcbor_simple_decode(state_d, &simple2), NULL);
	zassert_true(zcbor_simple_expect(state_d, 14), NULL);
	zassert_true(zcbor_nil_expect(state_d, NULL), NULL);
	zassert_false(zcbor_undefined_expect(state_d, NULL), NULL);
	zassert_true(zcbor_simple_expect(state_d, 24), NULL);
	zassert_false(zcbor_simple_expect(state_d, 254), NULL);
	zassert_true(zcbor_simple_decode(state_d, &simple1), NULL);
	zassert_equal(0, simple2, NULL);
	zassert_equal(255, simple1, NULL);
}


ZTEST(zcbor_unit_tests, test_header_len)
{
	zassert_equal(1, zcbor_header_len(0), NULL);
	zassert_equal(1, zcbor_header_len(23), NULL);
	zassert_equal(2, zcbor_header_len(24), NULL);
	zassert_equal(2, zcbor_header_len(0xFF), NULL);
	zassert_equal(3, zcbor_header_len(0x100), NULL);
	zassert_equal(3, zcbor_header_len(0xFFFF), NULL);
	zassert_equal(5, zcbor_header_len(0x10000), NULL);
	zassert_equal(5, zcbor_header_len(0xFFFFFFFF), NULL);
#if SIZE_MAX >= 0x100000000ULL
	zassert_equal(9, zcbor_header_len(0x100000000), NULL);
	zassert_equal(9, zcbor_header_len(0xFFFFFFFFFFFFFFFF), NULL);
#endif
}


ZTEST(zcbor_unit_tests, test_compare_strings)
{
	const uint8_t hello[] = "hello";
	const uint8_t hello2[] = "hello";
	const uint8_t long_str[] = "This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. ";
	const uint8_t long_str2[] = "This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. "
		"This is a very long string. This is a very long string. This is a very long string. ";
	struct zcbor_string test_str1_hello = {hello, 5};
	struct zcbor_string test_str2_hello_short = {hello, 4};
	struct zcbor_string test_str3_hello_offset = {&hello[1], 4};
	struct zcbor_string test_str4_long = {long_str, sizeof(long_str)};
	struct zcbor_string test_str5_long2 = {long_str2, sizeof(long_str2)};
	struct zcbor_string test_str6_empty = {hello, 0};
	struct zcbor_string test_str7_hello2 = {hello2, 5};
	struct zcbor_string test_str8_empty = {hello2, 0};
	struct zcbor_string test_str9_NULL = {NULL, 0};

	zassert_true(zcbor_compare_strings(&test_str1_hello, &test_str1_hello), NULL);
	zassert_true(zcbor_compare_strings(&test_str1_hello, &test_str7_hello2), NULL);
	zassert_true(zcbor_compare_strings(&test_str4_long, &test_str5_long2), NULL);
	zassert_true(zcbor_compare_strings(&test_str6_empty, &test_str8_empty), NULL);

	zassert_false(zcbor_compare_strings(&test_str2_hello_short, &test_str7_hello2), NULL);
	zassert_false(zcbor_compare_strings(&test_str3_hello_offset, &test_str7_hello2), NULL);
	zassert_false(zcbor_compare_strings(&test_str1_hello, NULL), NULL);
	zassert_false(zcbor_compare_strings(NULL, &test_str5_long2), NULL);
	zassert_false(zcbor_compare_strings(&test_str6_empty, NULL), NULL);
	zassert_false(zcbor_compare_strings(&test_str1_hello, &test_str9_NULL), NULL);
	zassert_false(zcbor_compare_strings(&test_str9_NULL, &test_str5_long2), NULL);
}


ZTEST(zcbor_unit_tests, test_any_skip)
{
	uint8_t payload[200];
	ZCBOR_STATE_E(state_e, 1, payload, sizeof(payload), 0);
	ZCBOR_STATE_D(state_d, 0, payload, sizeof(payload), 10);
	size_t exp_elem_count = 10;

	zassert_true(zcbor_uint32_put(state_e, 10), NULL);
	zassert_true(zcbor_any_skip(state_d, NULL));
	zassert_equal(state_d->payload, state_e->payload, NULL);
	zassert_equal(state_d->elem_count, --exp_elem_count, NULL);

	zassert_true(zcbor_int64_put(state_e, -10000000000000), NULL);
	zassert_true(zcbor_any_skip(state_d, NULL));
	zassert_equal(state_d->payload, state_e->payload, NULL);
	zassert_equal(state_d->elem_count, --exp_elem_count, NULL);

	zassert_true(zcbor_bstr_put_term(state_e, "hello"), NULL);
	zassert_true(zcbor_any_skip(state_d, NULL));
	zassert_equal(state_d->payload, state_e->payload, NULL);
	zassert_equal(state_d->elem_count, --exp_elem_count, NULL);

	zassert_true(zcbor_tstr_put_term(state_e, "world"), NULL);
	zassert_true(zcbor_any_skip(state_d, NULL));
	zassert_equal(state_d->payload, state_e->payload, NULL);
	zassert_equal(state_d->elem_count, --exp_elem_count, NULL);

	zassert_true(zcbor_nil_put(state_e, NULL), NULL);
	zassert_true(zcbor_any_skip(state_d, NULL));
	zassert_equal(state_d->payload, state_e->payload, NULL);
	zassert_equal(state_d->elem_count, --exp_elem_count, NULL);

	zassert_true(zcbor_float64_put(state_e, 3.14), NULL);
	zassert_true(zcbor_any_skip(state_d, NULL));
	zassert_equal(state_d->payload, state_e->payload, NULL);
	zassert_equal(state_d->elem_count, --exp_elem_count, NULL);

	zassert_true(zcbor_list_start_encode(state_e, 6), NULL);
	zassert_true(zcbor_uint32_put(state_e, 10), NULL);
	zassert_true(zcbor_int64_put(state_e, -10000000000000), NULL);
	zassert_true(zcbor_bstr_put_term(state_e, "hello"), NULL);
	zassert_true(zcbor_tstr_put_term(state_e, "world"), NULL);
	zassert_true(zcbor_bool_put(state_e, true), NULL);
	zassert_true(zcbor_float64_put(state_e, 3.14), NULL);
	zassert_true(zcbor_list_end_encode(state_e, 6), NULL);
	zassert_true(zcbor_any_skip(state_d, NULL));
	zassert_equal(state_d->payload, state_e->payload, NULL);
	zassert_equal(state_d->elem_count, --exp_elem_count, NULL);

	zassert_true(zcbor_tag_encode(state_e, 1), NULL);
	zassert_true(zcbor_tag_encode(state_e, 200), NULL);
	zassert_true(zcbor_tag_encode(state_e, 3000), NULL);
	zassert_true(zcbor_map_start_encode(state_e, 6), NULL);
	zassert_true(zcbor_uint32_put(state_e, 10), NULL);
	zassert_true(zcbor_int64_put(state_e, -10000000000000), NULL);
	zassert_true(zcbor_bstr_put_term(state_e, "hello"), NULL);
	zassert_true(zcbor_tstr_put_term(state_e, "world"), NULL);
	zassert_true(zcbor_undefined_put(state_e, NULL), NULL);
	zassert_true(zcbor_float64_put(state_e, 3.14), NULL);
	zassert_true(zcbor_map_end_encode(state_e, 6), NULL);
	zassert_true(zcbor_any_skip(state_d, NULL));
	zassert_equal(state_d->payload, state_e->payload, "0x%x != 0x%x\n",
		state_d->payload, state_e->payload);
	zassert_equal(state_d->elem_count, --exp_elem_count, NULL);
}

ZTEST_SUITE(zcbor_unit_tests, NULL, NULL, NULL, NULL, NULL);
