/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <zcbor_common.h>
#include <zcbor_decode.h>
#include <zcbor_print.h>


void zcbor_print_compare_lines(const uint8_t *str1, const uint8_t *str2, size_t size)
{
	for (size_t j = 0; j < size; j++) {
		zcbor_do_print("%x ", str1[j]);
	}
	zcbor_do_print("\r\n");
	for (size_t j = 0; j < size; j++) {
		zcbor_do_print("%x ", str2[j]);
	}
	zcbor_do_print("\r\n");
	for (size_t j = 0; j < size; j++) {
		zcbor_do_print("%x ", str1[j] != str2[j]);
	}
	zcbor_do_print("\r\n");
	zcbor_do_print("\r\n");
}


void zcbor_print_compare_strings(const uint8_t *str1, const uint8_t *str2, size_t size)
{
	const size_t col_width = 16;

	for (size_t i = 0; i <= size / col_width; i++) {
		zcbor_do_print("line %zu (char %zu)\r\n", i, i*col_width);
		zcbor_print_compare_lines(&str1[i*col_width], &str2[i*col_width],
			MIN(col_width, (size - i*col_width)));
	}
	zcbor_do_print("\r\n");
}


void zcbor_print_compare_strings_diff(const uint8_t *str1, const uint8_t *str2, size_t size)
{
	const size_t col_width = 16;
	bool printed = false;

	for (size_t i = 0; i <= size / col_width; i++) {
		if (memcmp(&str1[i*col_width], &str2[i*col_width], MIN(col_width, (size - i*col_width))) != 0) {
			zcbor_do_print("line %zu (char %zu)\r\n", i, i*col_width);
			zcbor_print_compare_lines(&str1[i*col_width], &str2[i*col_width],
				MIN(col_width, (size - i*col_width)));
			printed = true;
		}
	}
	if (printed) {
		zcbor_do_print("\r\n");
	}
}


const char *zcbor_error_str(int error)
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
		ZCBOR_ERR_CASE(ZCBOR_ERR_PAYLOAD_OUTDATED)
		ZCBOR_ERR_CASE(ZCBOR_ERR_ELEM_NOT_FOUND)
		ZCBOR_ERR_CASE(ZCBOR_ERR_MAP_MISALIGNED)
		ZCBOR_ERR_CASE(ZCBOR_ERR_ELEMS_NOT_PROCESSED)
		ZCBOR_ERR_CASE(ZCBOR_ERR_NOT_AT_END)
		ZCBOR_ERR_CASE(ZCBOR_ERR_MAP_FLAGS_NOT_AVAILABLE)
		ZCBOR_ERR_CASE(ZCBOR_ERR_INVALID_VALUE_ENCODING)
		ZCBOR_ERR_CASE(ZCBOR_ERR_CONSTANT_STATE_MISSING)
		ZCBOR_ERR_CASE(ZCBOR_ERR_BAD_ARG)
	}
	#undef ZCBOR_ERR_CASE

	return "ZCBOR_ERR_UNKNOWN";
}


void zcbor_print_error(int error)
{
	zcbor_do_print("%s\r\n", zcbor_error_str(error));
}
