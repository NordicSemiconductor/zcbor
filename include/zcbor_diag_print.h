/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZCBOR_DIAG_PRINT_H__
#define ZCBOR_DIAG_PRINT_H__


#ifdef __cplusplus
extern "C" {
#endif

#ifndef ZCBOR_DIAG_PRINT_FUNC
#include <zephyr/sys/printk.h>
#define zcbor_do_diag_print(...) printk(__VA_ARGS__)
#else
#define zcbor_do_diag_print(...) ZCBOR_DIAG_PRINT_FUNC(__VA_ARGS__)
#endif

static bool indent_printed = false;

__attribute__((used))
static void zcbor_print_indent(uint8_t indent_len)
{
	if (!indent_printed) {
		for (int i = 0; i < indent_len; i++) {
			zcbor_do_diag_print("| ");
		}
		indent_printed = true;
	}
}

__attribute__((used))
static void zcbor_print_newline(void)
{
	zcbor_do_diag_print("\r\n");
	indent_printed = false;
}

#define STR_COL_WIDTH 16

__attribute__((used))
static void zcbor_print_str(const uint8_t *str, size_t len, uint8_t indent_len)
{
	for (size_t i = 0; i < len; i++) {
		if (!(i % STR_COL_WIDTH)) {
			if (i > 0) {zcbor_print_newline();}
			zcbor_print_indent(indent_len);
			zcbor_do_diag_print("0x");
		}
		zcbor_do_diag_print("%02x ", str[i]);
	}
}

__attribute__((used))
static void zcbor_print_bstr(uint8_t const *payload, size_t len, uint8_t indent_len)
{
	zcbor_print_str(payload, len, indent_len);
	zcbor_print_newline();
}

static void zcbor_print_tstr(uint8_t const *payload, size_t len, uint8_t indent_len);

__attribute__((used))
static void zcbor_print_btstr(uint8_t const *payload, size_t len, uint8_t indent_len, bool is_bstr)
{
	if (is_bstr) {
		zcbor_print_bstr(payload, len, indent_len);
	} else {
		zcbor_print_tstr(payload, len, indent_len);
	}
}


#ifdef ZCBOR_DIAG_PRINT_PRETTY
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_common.h"
#define RESET_COLOR   "\x1B[0m"

#ifndef ZCBOR_DIAG_PRINT_COLOR_HEADER
#define ZCBOR_DIAG_PRINT_COLOR_HEADER "\x1B[31m" /* red */
#endif
#ifndef ZCBOR_DIAG_PRINT_COLOR_VALUE
#define ZCBOR_DIAG_PRINT_COLOR_VALUE "\x1B[34m" /* blue */
#endif
#ifndef ZCBOR_DIAG_PRINT_COLOR_DESC
#define ZCBOR_DIAG_PRINT_COLOR_DESC "\x1B[32m" /* green */
#endif
#ifndef ZCBOR_DIAG_PRINT_COLOR_TAG
#define ZCBOR_DIAG_PRINT_COLOR_TAG "\x1B[33m" /* yellow */
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
	zcbor_do_diag_print("%f", dvalue);
	return true;
}

__attribute__((used))
static void zcbor_print_numeric(uint8_t header_byte, uint64_t value)
{
	zcbor_major_type_t major_type = ZCBOR_MAJOR_TYPE(header_byte);
	uint8_t additional = ZCBOR_ADDITIONAL(header_byte);

	if (major_type == ZCBOR_MAJOR_TYPE_NINT) {
		zcbor_do_diag_print("-%llu", value + 1);
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
		zcbor_do_diag_print("%llu", value);
	}
}

__attribute__((used))
static void zcbor_print_value(uint8_t const *payload, size_t len, uint64_t value, uint8_t indent_len)
{
	uint8_t header_byte = *payload;

	zcbor_print_indent(indent_len);
	zcbor_do_diag_print(ZCBOR_DIAG_PRINT_COLOR_HEADER "0x%02x " ZCBOR_DIAG_PRINT_COLOR_VALUE,
			header_byte);
	if (len > 0) {
		zcbor_print_str(payload + 1, len - 1, 0);
	}
	zcbor_do_diag_print(ZCBOR_DIAG_PRINT_COLOR_DESC "(%s", zcbor_header_byte_str(header_byte));
	zcbor_print_numeric(header_byte, value);
	zcbor_do_diag_print(")" RESET_COLOR);
	zcbor_print_newline();
}

__attribute__((used))
static void zcbor_print_tstr(uint8_t const *payload, size_t len, uint8_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_diag_print("\"");
	size_t prev_i = 0;

	for (size_t i = 0; i < len; i++) {
		if (payload[i] == '\n') {
			/* Add indent after newlines. */
			zcbor_do_diag_print("%.*s", i - prev_i, payload + prev_i);
			prev_i = i + 1;
			zcbor_print_newline();
			zcbor_print_indent(indent_len);
		}
	}
	zcbor_do_diag_print("%.*s\"", len - prev_i, payload + prev_i);
	zcbor_print_newline();
}

__attribute__((used))
static void zcbor_print_tag(uint32_t tag, uint8_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_diag_print(ZCBOR_DIAG_PRINT_COLOR_TAG "0x%02x ", tag);
}

__attribute__((used))
static void zcbor_print_end(zcbor_major_type_t major_type, uint8_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_diag_print(ZCBOR_DIAG_PRINT_COLOR_HEADER "0xff " ZCBOR_DIAG_PRINT_COLOR_DESC "(%s end)" RESET_COLOR,
		zcbor_header_byte_strings[major_type]);
	zcbor_print_newline();
}

#else

__attribute__((used))
static void zcbor_print_tstr(uint8_t const *payload, size_t len, uint8_t indent_len)
{
	zcbor_do_diag_print("\"%.*s\"", len, payload);
	zcbor_print_newline();
}

__attribute__((used))
static void zcbor_print_value(uint8_t const *payload, size_t len, uint64_t value, uint8_t indent_len)
{
	zcbor_print_str(payload, len, indent_len);
	if (len) {
		if (ZCBOR_ADDITIONAL(*payload) == ZCBOR_VALUE_IS_INDEFINITE_LENGTH) {
			zcbor_do_diag_print("(start)");
		} else {
			zcbor_do_diag_print("(%llu)", value);
		}
		zcbor_print_newline();
	}
}

__attribute__((used))
static void zcbor_print_tag(uint32_t tag, uint8_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_diag_print("0x%02x ", tag);
}

__attribute__((used))
static void zcbor_print_end(zcbor_major_type_t major_type, uint8_t indent_len)
{
	zcbor_print_indent(indent_len);
	zcbor_do_diag_print("0xff (end)");
	zcbor_print_newline();
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* ZCBOR_DIAG_PRINT_H__ */
