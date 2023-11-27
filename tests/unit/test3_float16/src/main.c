/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef ZCBOR_VERBOSE

#include <zephyr/ztest.h>

ZTEST(zcbor_unit_tests3, test_skip)
{
	printf("Skip on VERBOSE builds because of print output volume.\n");
}

#else /* ZCBOR_VERBOSE */

#include <zephyr/ztest.h>
#include "zcbor_decode.h"
#include "zcbor_encode.h"

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

#define F16_MANTISSA_MSK 0x3FF /* Bitmask for the mantissa. */

ZTEST(zcbor_unit_tests3, test_float16_decode)
{
#ifdef ZCBOR_BIG_ENDIAN
	float *fps = ((float *)_binary_fp_bytes_decode_bin_start) + 0x10000;
#else
	float *fps = (float *)_binary_fp_bytes_decode_bin_start;
#endif
	uint8_t payload[3] = {0xf9, 0, 0};

	zassert_equal((size_t)&_binary_fp_bytes_decode_bin_size, 0x10000 * sizeof(float) * 2,
			"0x%x != 0x%x", (size_t)&_binary_fp_bytes_decode_bin_size,
			0x10000 * sizeof(float));

	for (int i = 0; i <= 0xFFFF; i++) {
		uint16_t i_be = i;
#ifndef ZCBOR_BIG_ENDIAN
		i_be = switch_bytes(i);
#endif
		*(uint16_t *)&payload[1] = i_be;

		float out;
		ZCBOR_STATE_D(state_d, 0, payload, sizeof(payload), 1, 0);
		ZCBOR_STATE_E(state_e, 0, payload, sizeof(payload), 0);

		zassert_true(zcbor_float16_decode(state_d, &out), NULL);
		zassert_true(zcbor_float16_put(state_e, out));


		uint16_t payload_ne = *(uint16_t *)&payload[1];
#ifndef ZCBOR_BIG_ENDIAN
		payload_ne = switch_bytes(payload_ne);
#endif

		if (isnan(fps[i])) {
			zassert_true(isnan(out), "Expected NaN (i = %d)\n", i);
			/* Mask out mantissa when comparing NaNs. */
			zassert_equal(payload_ne & ~F16_MANTISSA_MSK, i & ~F16_MANTISSA_MSK,
					"0x%04x != 0x%04x", payload_ne, i);
			/* Just check that mantissa is non-zero. */
			zassert_true(payload_ne & F16_MANTISSA_MSK, NULL);
		} else {
			zassert_equal(out, fps[i], "Failed i 0x%x (0x%x != 0x%x)\n",
					i, *(uint32_t *)&out, *(uint32_t *)&fps[i]);
			zassert_equal(payload_ne, i,
					"0x%04x != 0x%04x", payload_ne, i);
		}
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
#ifdef ZCBOR_BIG_ENDIAN
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

void print_percent(uint32_t i)
{
	if ((i % 0x800000) == 0) {
		static const uint8_t move_cursor_up[] = {0x1b, 0x5b, 0x41, 0};
		printf("%s", move_cursor_up);
		printf("\r%d%%\n", (int)((double)(i) / 0x80000000 * 100));
	}
}


/* Test a single value, and its negative counterpart. */
static void do_test(uint32_t i, uint16_t exp_value)
{
	uint32_t i2 = i + 0x80000000; /* Negative i */

	/* Reinterpret as floats. */
	float in = *(float *)&i;
	float in2 = *(float *)&i2;

	uint16_t out = zcbor_float32_to_16(in);
	uint16_t out2 = zcbor_float32_to_16(in2);

	print_percent(i);

	/* Check positive and negative. */
	zassert_true((out == exp_value)	&& (out2 == (exp_value + 0x8000)),
			"Failed i 0x%x (0x%x != 0x%x) or (0x%x != 0x%x)\n", i,
			out, exp_value, out2, exp_value + 0x8000);
}


static void do_test_nan(uint32_t i)
{
	float in = *(float *)&i;
	uint32_t i2 = i + 0x80000000;
	float in2 = *(float *)&i2;

	print_percent(i);

	uint16_t out = zcbor_float32_to_16(in);
	uint16_t out2 = zcbor_float32_to_16(in2);
	float out3 = zcbor_float16_to_32(out);
	float out4 = zcbor_float16_to_32(out2);

	zassert_true(isnan(out3), "Expected NaN: 0x%x (i = 0x%x)\n", out, i);
	zassert_true(isnan(out4), "Expected NaN: 0x%x (i = 0x%x)\n", out2, i2);
}


ZTEST(zcbor_unit_tests3, test_float16_encode)
{
	zassert_equal((size_t)&_binary_fp_bytes_encode_bin_size, 31742 * 4 * 2, NULL);
	printf("\n");

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
