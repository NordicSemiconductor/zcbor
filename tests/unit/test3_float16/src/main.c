/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef ZCBOR_VERBOSE

#include <zephyr/ztest.h>

ZTEST(zcbor_unit_tests3, test_skip)
{
	printk("Skip on VERBOSE builds because of print output volume.\n");
}

#else /* ZCBOR_VERBOSE */

#include <zephyr/ztest.h>
#include "zcbor_decode.h"
#include "zcbor_encode.h"
#include "zcbor_debug.h"

#include <math.h>

__attribute__((used))
static uint16_t switch_bytes(uint16_t in)
{
	return (in >> 8) | (in << 8);
}

/* These are created by
 * ld -r -b binary -o fp_bytes_decode.o fp_bytes_decode.bin */
extern uint8_t _binary_fp_bytes_decode_bin_start[];
extern uint8_t _binary_fp_bytes_decode_bin_end[];
extern uint8_t _binary_fp_bytes_decode_bin_size[];

ZTEST(zcbor_unit_tests3, test_float16_decode)
{
#ifdef ZCBOR_BIG_ENDIAN
	float *fps = ((float *)_binary_fp_bytes_decode_bin_start) + 0x10000;
#else
	float *fps = (float *)_binary_fp_bytes_decode_bin_start;
#endif
	uint8_t payload[3] = {0xf9, 0, 0};

	zassert_equal((size_t)&_binary_fp_bytes_decode_bin_size, 0x10000 * sizeof(float) * 2, "0x%x != 0x%x", (size_t)&_binary_fp_bytes_decode_bin_size, 0x10000 * sizeof(float));

	for (int i = 0; i <= 0xFFFF; i++) {
		uint16_t i_be = i;
#ifndef ZCBOR_BIG_ENDIAN
		i_be = switch_bytes(i);
#endif
		*(uint16_t *)&payload[1] = i_be;

		float out;
		ZCBOR_STATE_D(state_d, 0, payload, sizeof(payload), 1);

		zassert_true(zcbor_float16_decode(state_d, &out), NULL);
		if (isnan(fps[i])) {
			zassert_true(isnan(out), "Expected NaN (i = %d)\n", i);
		} else {
			zassert_equal(out, fps[i], "Failed i 0x%x (0x%x != 0x%x)\n", i, *(uint32_t *)&out, *(uint32_t *)&fps[i]);
		}

		ZCBOR_STATE_E(state_e, 0, payload, sizeof(payload), 0);

		zassert_true(zcbor_float16_put(state_e, out));
		zassert_equal(*(uint16_t *)&payload[1], i_be, NULL);
	}
}


/* These are created by
 * ld -r -b binary -o fp_bytes_encode.o fp_bytes_encode.bin */
extern uint8_t _binary_fp_bytes_encode_bin_start[];
extern uint8_t _binary_fp_bytes_encode_bin_end[];
extern uint8_t _binary_fp_bytes_encode_bin_size[];

/**
 * fp_bytes_encode.bin contains results from conversion from float32 to float16.
 * As an optimization, fp_bytes_encode.bin contains only values for the positive
 * nonzero_finite part.

 * positive:
 * zero:           0x00000000 0x33000000
 * nonzero finite: 0x33000001 0x477fefff
 * infinite:       0x477ff000 0x7f800000
 * nan:            0x7f800001 0x7fffffff
 *
 * negative:
 * zero:           0x80000000 0xb3000000
 * nonzero finite: 0xb3000001 0xc77fefff
 * infinite:       0xc77ff000 0xff800000
 * nan:            0xff800001 0xffffffff
*/
#define START_ZERO           0x00000000
#define START_NONZERO_FINITE 0x33000001
#define START_INFINITE       0x477ff000
#define START_NAN            0x7f800001

/* Return float16 results from fp_bytes_encode.bin */
static uint16_t get_fp_bytes()
{
#ifdef CONFIG_BIG_ENDIAN
	uint32_t *fps = ((uint32_t *)_binary_fp_bytes_encode_bin_start) + 31742;
#else
	uint32_t *fps = (uint32_t *)_binary_fp_bytes_encode_bin_start;
#endif
	static uint16_t retval = 0;
	static uint32_t i = 0;

	if (i++ == fps[retval]) {
		retval++;
	}

	return retval + 1;
}


/* Test a single value, and its negative counterpart. */
static void do_test(uint32_t i, uint16_t exp_value)
{
	uint8_t payload[6];

	ZCBOR_STATE_E(state_e, 0, payload, sizeof(payload), 0);
	uint32_t i2 = i + 0x80000000; /* Negative i */

	/* Reinterpret as floats. */
	float in = *(float *)&i;
	float in2 = *(float *)&i2;

	zassert_true(zcbor_float16_put(state_e, in), NULL); /* Encode positive i */
	zassert_true(zcbor_float16_put(state_e, in2), NULL); /* Encode negative i (i2). */

#ifndef ZCBOR_BIG_ENDIAN
	*(uint16_t*)&payload[1] = switch_bytes(*(uint16_t*)&payload[1]);
	*(uint16_t*)&payload[4] = switch_bytes(*(uint16_t*)&payload[4]);
#endif

	if ((i % 0x800000) == 0) {
		static const uint8_t move_cursor_up[] = {0x1b, 0x5b, 0x41, 0};
		printk("%s", move_cursor_up);
		printk("\r%d%%\n", (int)((double)(i) / 0x80000000 * 100));
	}

	/* Check positive and negative. */
	zassert_true((*(uint16_t*)&payload[1] == exp_value)
				&& (*(uint16_t*)&payload[4] == exp_value + 0x8000),
			"Failed i 0x%x (0x%x != 0x%x) or (0x%x != 0x%x)\n", i,
			*(uint16_t*)&payload[1], exp_value,
			*(uint16_t*)&payload[4], exp_value + 0x8000);
}


static void do_test_nan(uint32_t i)
{
	uint8_t payload[6];
	ZCBOR_STATE_E(state_e, 0, payload, sizeof(payload), 0);
	ZCBOR_STATE_D(state_d, 0, payload, sizeof(payload), 2);
	float in = *(float *)&i;
	uint32_t i2 = i + 0x80000000;
	float in2 = *(float *)&i2;
	float out;
	float out2;

	if (((i + 1) % 0x800000) == 0) {
		static const uint8_t move_cursor_up[] = {0x1b, 0x5b, 0x41, 0};
		printk("%s", move_cursor_up);
		printk("\r%d%%\n", (int)((double)(i + 1) / 0x80000000 * 100));
	}

	zassert_true(zcbor_float16_put(state_e, in), NULL);
	zassert_true(zcbor_float16_put(state_e, in2), NULL);
	zassert_true(zcbor_float16_decode(state_d, &out), NULL);
	zassert_true(zcbor_float16_decode(state_d, &out2), NULL);
	zassert_true(isnan(out), "Expected NaN: 0x%x (i = 0x%x)\n", switch_bytes(*(uint16_t *)&payload[1]), i);
	zassert_true(isnan(out2), "Expected NaN: 0x%x (i = 0x%x)\n", switch_bytes(*(uint16_t *)&payload[4]), i2);
}


ZTEST(zcbor_unit_tests3, test_float16_encode)
{
	zassert_equal((size_t)&_binary_fp_bytes_encode_bin_size, 31742 * 4 * 2, NULL);
	printk("\n");

	for (uint32_t i = START_ZERO; i < START_NONZERO_FINITE; i++) {
		do_test(i, 0);
	}
	for (uint32_t i = START_NONZERO_FINITE; i < START_INFINITE; i++) {
		do_test(i, get_fp_bytes());
	}
	for (uint32_t i = START_INFINITE; i < START_NAN; i++) {
		do_test(i, 0x7c00);
	}
	for (uint32_t i = START_NAN; i < 0x80000000; i++) {
		do_test_nan(i);
	}
}

#endif /* ZCBOR_VERBOSE */

ZTEST_SUITE(zcbor_unit_tests3, NULL, NULL, NULL, NULL, NULL);
