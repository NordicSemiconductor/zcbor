/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZCBOR_PRINT_H__
#define ZCBOR_PRINT_H__


#ifdef __cplusplus
extern "C" {
#endif

#ifndef ZCBOR_PRINT_FUNC
#include <zephyr/sys/printk.h>
#define zcbor_do_print(...) printk(__VA_ARGS__)
#else
#define zcbor_do_print(...) ZCBOR_PRINT_FUNC(__VA_ARGS__)
#endif

#ifdef ZCBOR_VERBOSE
#define zcbor_trace() (zcbor_do_print("bytes left: %zu, byte: 0x%x, elem_count: 0x%zx, err: %d, %s:%d\n",\
	(size_t)state->payload_end - (size_t)state->payload, *state->payload, state->elem_count, \
	state->constant_state ? state->constant_state->error : 0, __FILE__, __LINE__))

#define zcbor_print_assert(expr, ...) \
do { \
	zcbor_do_print("ASSERTION \n  \"" #expr \
		"\"\nfailed at %s:%d with message:\n  ", \
		__FILE__, __LINE__); \
	zcbor_do_print(__VA_ARGS__);\
} while(0)
#define zcbor_print(...) zcbor_do_print(__VA_ARGS__)
#else
#define zcbor_trace() ((void)state)
#define zcbor_print_assert(...)
#define zcbor_print(...)
#endif

#ifdef ZCBOR_ASSERTS
#define zcbor_assert(expr, ...) \
do { \
	if (!(expr)) { \
		zcbor_print_assert(expr, __VA_ARGS__); \
		ZCBOR_FAIL(); \
	} \
} while(0)
#define zcbor_assert_state(expr, ...) \
do { \
	if (!(expr)) { \
		zcbor_print_assert(expr, __VA_ARGS__); \
		ZCBOR_ERR(ZCBOR_ERR_ASSERTION); \
	} \
} while(0)
#else
#define zcbor_assert(expr, ...)
#define zcbor_assert_state(expr, ...)
#endif

__attribute__((used))
static void zcbor_print_compare_lines(const uint8_t *str1, const uint8_t *str2, uint32_t size)
{
	for (uint32_t j = 0; j < size; j++) {
		zcbor_do_print("%x ", str1[j]);
	}
	zcbor_do_print("\r\n");
	for (uint32_t j = 0; j < size; j++) {
		zcbor_do_print("%x ", str2[j]);
	}
	zcbor_do_print("\r\n");
	for (uint32_t j = 0; j < size; j++) {
		zcbor_do_print("%x ", str1[j] != str2[j]);
	}
	zcbor_do_print("\r\n");
	zcbor_do_print("\r\n");
}

__attribute__((used))
static void zcbor_print_compare_strings(const uint8_t *str1, const uint8_t *str2, uint32_t size)
{
	for (uint32_t i = 0; i <= size / 16; i++) {
		zcbor_do_print("line %d (char %d)\r\n", i, i*16);
		zcbor_print_compare_lines(&str1[i*16], &str2[i*16],
			MIN(16, (size - i*16)));
	}
	zcbor_do_print("\r\n");
}

__attribute__((used))
static void zcbor_print_compare_strings_diff(const uint8_t *str1, const uint8_t *str2, uint32_t size)
{
	bool printed = false;
	for (uint32_t i = 0; i <= size / 16; i++) {
		if (memcmp(&str1[i*16], &str2[i*16], MIN(16, (size - i*16)) != 0)) {
			zcbor_do_print("line %d (char %d)\r\n", i, i*16);
			zcbor_print_compare_lines(&str1[i*16], &str2[i*16],
				MIN(16, (size - i*16)));
			printed = true;
		}
	}
	if (printed) {
		zcbor_do_print("\r\n");
	}
}

__attribute__((used))
static const char *zcbor_error_str(int error)
{
	#define ZCBOR_ERR_CASE(err) case err: \
		return #err; /* The literal is static per C99 6.4.5 paragraph 5. */\

	switch(error) {
		ZCBOR_ERR_CASE(ZCBOR_SUCCESS)
		ZCBOR_ERR_CASE(ZCBOR_ERR_NO_BACKUP_MEM)
		ZCBOR_ERR_CASE(ZCBOR_ERR_NO_BACKUP_ACTIVE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_LOW_ELEM_COUNT)
		ZCBOR_ERR_CASE(ZCBOR_ERR_HIGH_ELEM_COUNT)
		ZCBOR_ERR_CASE(ZCBOR_ERR_INT_SIZE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_FLOAT_SIZE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_ADDITIONAL_INVAL)
		ZCBOR_ERR_CASE(ZCBOR_ERR_NO_PAYLOAD)
		ZCBOR_ERR_CASE(ZCBOR_ERR_PAYLOAD_NOT_CONSUMED)
		ZCBOR_ERR_CASE(ZCBOR_ERR_WRONG_TYPE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_WRONG_VALUE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_WRONG_RANGE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_ITERATIONS)
		ZCBOR_ERR_CASE(ZCBOR_ERR_ASSERTION)
	}
	#undef ZCBOR_ERR_CASE

	return "ZCBOR_ERR_UNKNOWN";
}

__attribute__((used))
static void zcbor_print_error(int error)
{
	zcbor_do_print("%s\r\n", zcbor_error_str(error));
}

#ifdef __cplusplus
}
#endif

#endif /* ZCBOR_PRINT_H__ */
