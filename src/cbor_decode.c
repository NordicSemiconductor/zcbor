/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "cbor_decode.h"

static uint32_t additional_len(uint8_t additional)
{
	switch (additional) {
	case 24: return 1;
	case 25: return 2;
	case 26: return 4;
	case 27: return 8;
	default: return 0;
	}
}

#define COMP_PTRS(type, p_res, p_min, p_max) \
		(((p_min == NULL) || (*(type *)p_res >= *(type *)p_min)) \
		&& ((p_max == NULL) || (*(type *)p_res <= *(type *)p_max)))

static bool value_extract(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		void * const p_result, size_t result_len)
{
	uint8_t *p_u8_result  = (uint8_t *)p_result;
	uint8_t additional = **pp_payload & 0x1F;
	(*pp_payload)++;

	cbor_decode_trace();

	if (result_len == 0 || *p_elem_count == 0) {
		cbor_decode_trace();
		(*pp_payload)--;
		return false;
	}

	memset(p_result, 0, result_len);
#ifdef CONFIG_BIG_ENDIAN
	p_u8_result[result_len - 1] = additional;
#else
	p_u8_result[0] = additional;
#endif /* CONFIG_BIG_ENDIAN */
	if (additional > 23) {
		uint32_t len = additional_len(additional);

		if (len > result_len) {
			cbor_decode_trace();
			(*pp_payload)--;
			return false;
		}
#ifdef CONFIG_BIG_ENDIAN
		memcpy(&p_u8_result[result_len - len], *pp_payload, len);
#else
		for (uint32_t i = 0; i < len; i++) {
			p_u8_result[i] = (*pp_payload)[len - i - 1];
		}
#endif /* CONFIG_BIG_ENDIAN */
		(*pp_payload) += len;
	}

	if (*pp_payload > p_payload_end) {
		/* Value extended beyond the payload. */
		cbor_decode_trace();
		(*pp_payload)--;
		return false;
	}
	(*p_elem_count)--;
	return true;
}


static bool int32_decode(uint8_t const **const pp_payload,
			uint8_t const *const p_payload_end,
			size_t *const p_elem_count, int32_t *p_result,
			void *p_min_value, void *p_max_value)
{
	uint8_t major_type = **pp_payload >> 5;

	if (!value_extract(pp_payload, p_payload_end, p_elem_count, p_result,
			4)) {
		cbor_decode_trace();
		return false;
	}
	if (*p_result < 0) {
		/* Value is too large to fit in a signed integer. */
		cbor_decode_trace();
		return false;
	}

	if (major_type == CBOR_MAJOR_TYPE_NINT) {
		*p_result *= -1;
	}
	if (!COMP_PTRS(int32_t, p_result, p_min_value, p_max_value)) {
		cbor_decode_trace();
		return false;
	}
	cbor_decode_print("val: %d\r\n", *p_result);
	return true;
}


bool intx32_decode(uint8_t const **const pp_payload,
			uint8_t const *const p_payload_end,
			size_t *const p_elem_count, int32_t *p_result,
			void *p_min_value, void *p_max_value)
{
	uint8_t major_type = **pp_payload >> 5;

	if (major_type != CBOR_MAJOR_TYPE_PINT
		&& major_type != CBOR_MAJOR_TYPE_NINT) {
		/* Value to be read doesn't have the right type. */
		cbor_decode_trace();
		return false;
	}

	if (!int32_decode(pp_payload, p_payload_end, p_elem_count,
				p_result, p_min_value,
				p_max_value)){
		cbor_decode_trace();
		return false;
	}
	return true;
}


static bool uint32_decode(uint8_t const **const pp_payload,
			uint8_t const *const p_payload_end,
			size_t *const p_elem_count,
			void *p_result, void *p_min_value,
			void *p_max_value)
{
	if (!value_extract(pp_payload, p_payload_end, p_elem_count, p_result,
			4)) {
		cbor_decode_trace();
		return false;
	}

	if (!COMP_PTRS(uint32_t, p_result, p_min_value, p_max_value)) {
		cbor_decode_trace();
		return false;
	}
	cbor_decode_print("val: %u\r\n", *p_result);
	return true;
}


bool uintx32_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end,
		size_t *const p_elem_count,
		uint32_t *p_result, void *p_min_value,
		void *p_max_value)
{
	uint8_t major_type = **pp_payload >> 5;

	if (major_type != CBOR_MAJOR_TYPE_PINT) {
		/* Value to be read doesn't have the right type. */
		cbor_decode_trace();
		return false;
	}
	if (!uint32_decode(pp_payload, p_payload_end, p_elem_count,
				p_result, p_min_value, p_max_value)){
		cbor_decode_trace();
		return false;
	}
	return true;
}


bool size_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end,
		size_t *const p_elem_count,
		size_t *p_result, size_t *p_min_value,
		size_t *p_max_value)
{
	_Static_assert((sizeof(size_t) == sizeof(uint32_t)), "");
	return uint32_decode(pp_payload, p_payload_end, p_elem_count,
			p_result, p_min_value, p_max_value);
}


bool strx_start_decode(uint8_t const **const pp_payload,
				uint8_t const *const p_payload_end,
				size_t *const p_elem_count,
				void *p_result, void *p_min_len,
				void *p_max_len)
{
	uint8_t major_type = **pp_payload >> 5;
	cbor_string_type_t *p_str_result = (cbor_string_type_t *)p_result;

	if (major_type != CBOR_MAJOR_TYPE_BSTR
		&& major_type != CBOR_MAJOR_TYPE_TSTR) {
		/* Value to be read doesn't have the right type. */
		cbor_decode_trace();
		return false;
	}
	if (!size_decode(pp_payload, p_payload_end, p_elem_count,
			&p_str_result->len, (size_t *)p_min_len,
			(size_t *)p_max_len)) {
		cbor_decode_trace();
		return false;
	}
	p_str_result->value = *pp_payload;
	return true;
}

bool strx_decode(uint8_t const **const pp_payload,
				uint8_t const *const p_payload_end,
				size_t *const p_elem_count,
				void *p_result, void *p_min_len,
				void *p_max_len)
{
	if (!strx_start_decode(pp_payload, p_payload_end, p_elem_count, p_result,
				p_min_len, p_max_len)) {
		return false;
	}
	cbor_string_type_t *p_str_result = (cbor_string_type_t *)p_result;
	(*pp_payload) += p_str_result->len;
	return true;
}


bool list_start_decode(uint8_t const **const pp_payload,
			uint8_t const *const p_payload_end,
			size_t *const p_elem_count,
			size_t *p_result, size_t min_num, size_t max_num)
{
	uint8_t major_type = **pp_payload >> 5;

	if (major_type != CBOR_MAJOR_TYPE_LIST
		&& major_type != CBOR_MAJOR_TYPE_MAP) {
		cbor_decode_trace();
		return false;
	}
	if (!uint32_decode(pp_payload, p_payload_end, p_elem_count,
			p_result, &min_num, &max_num)) {
		cbor_decode_trace();
		return false;
	}
	return true;
}


bool primx_decode(uint8_t const **const pp_payload,
				uint8_t const *const p_payload_end,
				size_t *const p_elem_count,
				uint8_t *p_result, void *p_min_result,
				void *p_max_result)
{
	uint8_t major_type = **pp_payload >> 5;
	uint32_t val;


	if (major_type != CBOR_MAJOR_TYPE_PRIM) {
		/* Value to be read doesn't have the right type. */
		cbor_decode_trace();
		return false;
	}
	if (!uint32_decode(pp_payload, p_payload_end, p_elem_count,
			&val, p_min_result, p_max_result)) {
		cbor_decode_trace();
		return false;
	}
	if (p_result != NULL) {
		*p_result = val;
	}
	return true;
}


bool boolx_decode(uint8_t const **const pp_payload,
				uint8_t const *const p_payload_end,
				size_t *const p_elem_count,
				bool *p_result, void *p_min_result,
				void *p_max_result)
{
	uint8_t min_result = *(uint8_t *)p_min_result + 20;
	uint8_t max_result = *(uint8_t *)p_max_result + 20;

	if (!primx_decode(pp_payload, p_payload_end, p_elem_count,
			(uint8_t *)p_result, &min_result, &max_result)) {
		cbor_decode_trace();
		return false;
	}
	(*p_result) -= 20;
	return true;
}


bool double_decode(uint8_t const **const pp_payload,
				uint8_t const *const p_payload_end,
				size_t *const p_elem_count,
				double *p_result, void *p_min_result,
				void *p_max_result)
{
	uint8_t major_type = **pp_payload >> 5;

	if (major_type != CBOR_MAJOR_TYPE_PRIM) {
		/* Value to be read doesn't have the right type. */
		cbor_decode_trace();
		return false;
	}
	if (!value_extract(pp_payload, p_payload_end, p_elem_count, p_result,
			sizeof(*p_result))) {
		cbor_decode_trace();
		return false;
	}

	if (!COMP_PTRS(double, p_result, p_min_result, p_max_result)) {
		cbor_decode_trace();
		return false;
	}
	return true;
}


bool any_decode(uint8_t const **const pp_payload,
				uint8_t const *const p_payload_end,
				size_t *const p_elem_count,
				void *p_result, void *p_min_result,
				void *p_max_result)
{
	if (p_result != NULL) {
		/* 'any' type cannot be returned, only skipped. */
		cbor_decode_trace();
		return false;
	}

	uint8_t major_type = **pp_payload >> 5;
	uint64_t value;

	if (!value_extract(pp_payload, p_payload_end, p_elem_count, &value,
			8)) {
		/* Should not happen? */
		cbor_decode_trace();
		return false;
	}

	switch (major_type) {
		case CBOR_MAJOR_TYPE_BSTR:
		case CBOR_MAJOR_TYPE_TSTR:
		case CBOR_MAJOR_TYPE_LIST:
		case CBOR_MAJOR_TYPE_MAP:
			(*pp_payload) += value;
			break;
		default:
			/* Do nothing */
			break;
	}

	return true;
}


bool multi_decode(size_t min_decode, size_t max_decode,
				size_t *p_num_decode, decoder_t decoder,
				uint8_t const **const pp_payload,
				uint8_t const *const p_payload_end,
				size_t *const p_elem_count,
				void *p_result, void *p_min_result,
				void *p_max_result, size_t result_len)
{
	for (size_t i = 0; i < max_decode; i++) {
		uint8_t const *p_payload_bak = *pp_payload;
		size_t elem_count_bak = *p_elem_count;

		if (!decoder(pp_payload, p_payload_end, p_elem_count,
				(uint8_t *)p_result + i*result_len,
				p_min_result, p_max_result)) {
			*p_num_decode = i;
			if (i < min_decode) {
				cbor_decode_trace();
			} else {
				cbor_decode_print("Found %d elements.\n", i);
			}
			*pp_payload = p_payload_bak;
			*p_elem_count = elem_count_bak;
			return (i >= min_decode);
		}
	}
	cbor_decode_print("Found %d elements.\n", max_decode);
	*p_num_decode = max_decode;
	return true;
}
