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
#include <stdio.h>
#define zcbor_do_print(...) printf(__VA_ARGS__)
#else
#define zcbor_do_print(...) ZCBOR_PRINT_FUNC(__VA_ARGS__)
#endif

#ifdef ZCBOR_VERBOSE
#define zcbor_trace_raw(state) (zcbor_do_print("rem: %zu, cur: 0x%x, ec: 0x%zx, err: %d",\
	(size_t)state->payload_end - (size_t)state->payload, *state->payload, state->elem_count, \
	state->constant_state ? state->constant_state->error : 0))
#define zcbor_trace(state, appendix) do { \
	zcbor_trace_raw(state); \
	zcbor_do_print(", %s\n", appendix); \
} while(0)
#define zcbor_trace_file(state) do { \
	zcbor_trace_raw(state); \
	zcbor_do_print(", %s:%d\n", __FILE__, __LINE__); \
} while(0)

#define zcbor_log_assert(expr, ...) \
do { \
	zcbor_do_print("ASSERTION \n  \"" #expr \
		"\"\nfailed at %s:%d with message:\n  ", \
		__FILE__, __LINE__); \
	zcbor_do_print(__VA_ARGS__);\
} while(0)
#define zcbor_log(...) zcbor_do_print(__VA_ARGS__)
#else
#define zcbor_trace(state, appendix)
#define zcbor_trace_file(state) ((void)state)
#define zcbor_log_assert(...)
#define zcbor_log(...)
#endif

#ifdef ZCBOR_ASSERTS
#define zcbor_assert(expr, ...) \
do { \
	if (!(expr)) { \
		zcbor_log_assert(expr, __VA_ARGS__); \
		ZCBOR_FAIL(); \
	} \
} while(0)
#define zcbor_assert_state(expr, ...) \
do { \
	if (!(expr)) { \
		zcbor_log_assert(expr, __VA_ARGS__); \
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

#ifdef ZCBOR_PRINT_CBOR
static bool indent_printed = false;

__attribute__((used))
static void zcbor_print_indent(size_t indent_len)
{
	if (!indent_printed) {
		for (int i = 0; i < indent_len; i++) {
			zcbor_do_print("| ");
		}
		indent_printed = true;
	}
}

__attribute__((used))
static void zcbor_print_newline(void)
{
	zcbor_do_print("\r\n");
	indent_printed = false;
}

#define BYTES_PER_LINE 16

__attribute__((used))
static void zcbor_print_str(const uint8_t *str, size_t len, size_t indent_len)
{
	for (size_t i = 0; i < len; i++) {
		if (!(i % BYTES_PER_LINE)) {
			if (i > 0) {zcbor_print_newline();}
			zcbor_print_indent(indent_len);
			zcbor_do_print("0x");
		}
		zcbor_do_print("%02x ", str[i]);
	}
}

__attribute__((used))
static void zcbor_print_bstr(uint8_t const *payload, size_t len, size_t indent_len)
{
	zcbor_print_str(payload, len, indent_len);
	zcbor_print_newline();
}

static void zcbor_print_tstr(uint8_t const *payload, size_t len, size_t indent_len);

__attribute__((used))
static void zcbor_print_btstr(uint8_t const *payload, size_t len, bool is_bstr, size_t indent_len)
{
	if (is_bstr) {
		zcbor_print_bstr(payload, len, indent_len);
	} else {
		zcbor_print_tstr(payload, len, indent_len);
	}
}


#ifdef ZCBOR_PRINT_CBOR_PRETTY
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_common.h"
#define RESET_COLOR   "\x1B[0m"

#ifndef ZCBOR_PRINT_CBOR_COLOR_HEADER
#define ZCBOR_PRINT_CBOR_COLOR_HEADER "\x1B[31m" /* red */
#endif
#ifndef ZCBOR_PRINT_CBOR_COLOR_VALUE
#define ZCBOR_PRINT_CBOR_COLOR_VALUE "\x1B[34m" /* blue */
#endif
#ifndef ZCBOR_PRINT_CBOR_COLOR_DESC
#define ZCBOR_PRINT_CBOR_COLOR_DESC "\x1B[32m" /* green */
#endif
#ifndef ZCBOR_PRINT_CBOR_COLOR_TAG
#define ZCBOR_PRINT_CBOR_COLOR_TAG "\x1B[33m" /* yellow */
#endif


const static char *zcbor_header_byte_strings[] = {
	"", "", "bstr", "tstr", "list", "map", "tag", "simple"
};


__attribute__((used))
static const char *zcbor_header_byte_str(uint8_t header_byte)
{
	zcbor_major_type_t major_type = ZCBOR_MAJOR_TYPE(header_byte);
	uint8_t additional = ZCBOR_ADDITIONAL(header_byte);
	const char *simple_strings[] = {"false", "true", "nil", "undefined"};

	if ((major_type < ZCBOR_MAJOR_TYPE_SIMPLE)
			|| (additional < ZCBOR_BOOL_TO_SIMPLE)
			|| (additional == ZCBOR_VALUE_IS_1_BYTE)) {
		return zcbor_header_byte_strings[major_type];
	} else if (additional <= ZCBOR_VALUE_IN_HEADER) {
		return simple_strings[additional - ZCBOR_BOOL_TO_SIMPLE];
	}

	return "";
}

__attribute__((used))
static bool zcbor_print_float(int8_t additional, uint64_t value)
{
	uint32_t value32;
	double dvalue;

	switch(additional) {
	case ZCBOR_VALUE_IS_2_BYTES:
		dvalue = (double)zcbor_float16_to_32((uint16_t)value);
		break;
	case ZCBOR_VALUE_IS_4_BYTES:
		value32 = (uint32_t)value;
		dvalue = (double)(*(float *)&value32);
		break;
	case ZCBOR_VALUE_IS_8_BYTES:
		dvalue = *(double *)&value;
		break;
	default:
		return false;
	}
	zcbor_do_print("%f", dvalue);
	return true;
}

__attribute__((used))
static void zcbor_print_numeric(uint8_t header_byte, uint64_t value)
{
	zcbor_major_type_t major_type = ZCBOR_MAJOR_TYPE(header_byte);
	uint8_t additional = ZCBOR_ADDITIONAL(header_byte);

	if (major_type == ZCBOR_MAJOR_TYPE_NINT) {
		zcbor_do_print("-%llu", value + 1);
	} else if (major_type == ZCBOR_MAJOR_TYPE_SIMPLE
			&& additional >= ZCBOR_BOOL_TO_SIMPLE
			&& additional <= ZCBOR_VALUE_IN_HEADER) {
		/* Do nothing */
	} else if (additional == ZCBOR_VALUE_IS_INDEFINITE_LENGTH) {
		/* Do nothing */
	} else if (major_type == ZCBOR_MAJOR_TYPE_SIMPLE
			&& zcbor_print_float(additional, value)) {
		/* Do nothing, already printed */
	} else {
		zcbor_do_print("%llu", value);
	}
}

__attribute__((used))
static void zcbor_print_value(uint8_t const *payload, size_t len, uint64_t value, size_t indent_len)
{
	uint8_t header_byte = *payload;

	zcbor_print_indent(indent_len);
	zcbor_do_print(ZCBOR_PRINT_CBOR_COLOR_HEADER "0x%02x " ZCBOR_PRINT_CBOR_COLOR_VALUE,
			header_byte);
	if (len > 0) {
		zcbor_print_str(payload + 1, len - 1, 0);
	}
	zcbor_do_print(ZCBOR_PRINT_CBOR_COLOR_DESC "(%s", zcbor_header_byte_str(header_byte));
	zcbor_print_numeric(header_byte, value);
	zcbor_do_print(")" RESET_COLOR);
	zcbor_print_newline();
}

__attribute__((used))
static void zcbor_print_tstr(uint8_t const *payload, size_t len, size_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_print("\"");
	size_t prev_i = 0;

	for (size_t i = 0; i < len; i++) {
		if (payload[i] == '\n') {
			/* Add indent after newlines. */
			zcbor_do_print("%.*s", i - prev_i, payload + prev_i);
			prev_i = i + 1;
			zcbor_print_newline();
			zcbor_print_indent(indent_len);
		}
	}
	zcbor_do_print("%.*s\"", len - prev_i, payload + prev_i);
	zcbor_print_newline();
}

__attribute__((used))
static void zcbor_print_tag(uint32_t tag, size_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_print(ZCBOR_PRINT_CBOR_COLOR_TAG "0x%02x ", tag);
}

__attribute__((used))
static void zcbor_print_end(zcbor_major_type_t major_type, size_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_print(ZCBOR_PRINT_CBOR_COLOR_HEADER "0xff " ZCBOR_PRINT_CBOR_COLOR_DESC "(%s end)" RESET_COLOR,
		zcbor_header_byte_strings[major_type]);
	zcbor_print_newline();
}

#else

__attribute__((used))
static void zcbor_print_tstr(uint8_t const *payload, size_t len, size_t indent_len)
{
	zcbor_do_print("\"%.*s\"", len, payload);
	zcbor_print_newline();
}

__attribute__((used))
static void zcbor_print_value(uint8_t const *payload, size_t len, uint64_t value, size_t indent_len)
{
	zcbor_print_str(payload, len, indent_len);
	if (len) {
		if (ZCBOR_ADDITIONAL(*payload) == ZCBOR_VALUE_IS_INDEFINITE_LENGTH) {
			zcbor_do_print("(start)");
		} else {
			zcbor_do_print("(%llu)", value);
		}
		zcbor_print_newline();
	}
}

__attribute__((used))
static void zcbor_print_tag(uint32_t tag, size_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_print("0x%02x ", tag);
}

__attribute__((used))
static void zcbor_print_end(zcbor_major_type_t major_type, size_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_print("0xff (end)");
	zcbor_print_newline();
}

#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZCBOR_PRINT_H__ */
