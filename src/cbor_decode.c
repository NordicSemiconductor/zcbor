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
#include "cbor_common.h"


/** Return value length from additional value.
 */
static uint32_t additional_len(uint8_t additional)
{
	if (24 <= additional && additional <= 27) {
		/* 24 => 1
		 * 25 => 2
		 * 26 => 4
		 * 27 => 8
		 */
		return 1 << (additional - 24);
	}
	return 0;
}

/** Extract the major type, i.e. the first 3 bits of the header byte. */
#define MAJOR_TYPE(header_byte) (((header_byte) >> 5) & 0x7)

/** Extract the additional info, i.e. the last 5 bits of the header byte. */
#define ADDITIONAL(header_byte) ((header_byte) & 0x1F)


#define FAIL_AND_DECR_IF(expr) \
do {\
	if (expr) { \
		(p_state->p_payload)--; \
		FAIL(); \
	} \
} while(0)

/** Get a single value.
 *
 * @details @p pp_payload must point to the header byte. This function will
 *          retrieve the value (either from within the additional info, or from
 *          the subsequent bytes) and return it in the result. The result can
 *          have arbitrary length.
 *
 *          The function will also validate
 *           - Min/max constraints on the value.
 *           - That @p pp_payload doesn't overrun past @p p_payload_end.
 *           - That @p p_elem_count has not been exhausted.
 *
 *          @p pp_payload and @p p_elem_count are updated if the function
 *          succeeds. If not, they are left unchanged.
 *
 *          CBOR values are always big-endian, so this function converts from
 *          big to little-endian if necessary (@ref CONFIG_BIG_ENDIAN).
 */
static bool value_extract(cbor_state_t * p_state,
		void * const p_result, size_t result_len)
{
	cbor_trace();
	cbor_assert(result_len != 0, "0-length result not supported.\n");
	cbor_assert(p_result != NULL, NULL);

	FAIL_AND_DECR_IF(p_state->elem_count == 0);
	FAIL_AND_DECR_IF(p_state->p_payload >= p_state->p_payload_end);

	uint8_t *p_u8_result  = (uint8_t *)p_result;
	uint8_t additional = ADDITIONAL(*p_state->p_payload);

	(p_state->p_payload)++;

	memset(p_result, 0, result_len);
	if (additional <= VALUE_IN_HEADER) {
#ifdef CONFIG_BIG_ENDIAN
		p_u8_result[result_len - 1] = additional;
#else
		p_u8_result[0] = additional;
#endif /* CONFIG_BIG_ENDIAN */
	} else {
		uint32_t len = additional_len(additional);

		FAIL_AND_DECR_IF(len > result_len);
		FAIL_AND_DECR_IF((p_state->p_payload + len)
				> p_state->p_payload_end);

#ifdef CONFIG_BIG_ENDIAN
		memcpy(&p_u8_result[result_len - len], p_state->p_payload, len);
#else
		for (uint32_t i = 0; i < len; i++) {
			p_u8_result[i] = (p_state->p_payload)[len - i - 1];
		}
#endif /* CONFIG_BIG_ENDIAN */

		(p_state->p_payload) += len;
	}

	(p_state->elem_count)--;
	return true;
}


static bool int32_decode(cbor_state_t * p_state,
		int32_t *p_result, void *p_min_value, void *p_max_value)
{
	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);
	uint32_t uint_result;

	if (!value_extract(p_state, &uint_result, 4)) {
		FAIL();
	}

	cbor_print("uintval: %u\r\n", uint_result);
	if (uint_result >= (1 << (8*sizeof(uint_result)-1))) {
		/* Value is too large to fit in a signed integer. */
		FAIL();
	}

	if (major_type == CBOR_MAJOR_TYPE_NINT) {
		// Convert from CBOR's representation.
		*p_result = -1 - uint_result;
	} else {
		*p_result = uint_result;
	}

	cbor_print("val: %d\r\n", *p_result);
	if (!PTR_VALUE_IN_RANGE(int32_t, p_result, p_min_value, p_max_value)) {
		FAIL();
	}
	return true;
}


bool intx32_decode(cbor_state_t * p_state,
		int32_t *p_result, void *p_min_value, void *p_max_value)
{
	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);

	if (major_type != CBOR_MAJOR_TYPE_PINT
		&& major_type != CBOR_MAJOR_TYPE_NINT) {
		/* Value to be read doesn't have the right type. */
		FAIL();
	}

	if (!int32_decode(p_state,
				p_result, p_min_value,
				p_max_value)){
		FAIL();
	}
	return true;
}


static bool uint32_decode(cbor_state_t * p_state,
		void *p_result, void *p_min_value, void *p_max_value)
{
	if (!value_extract(p_state, p_result, 4)) {
		FAIL();
	}

	cbor_print("val: %u ", *(uint32_t *)p_result);
	if (!PTR_VALUE_IN_RANGE(uint32_t, p_result, p_min_value, p_max_value)) {
		cbor_print("Failed: ");
		if (p_min_value) cbor_print("min: %d ", *(uint32_t *)p_min_value);
		if (p_max_value) cbor_print("max: %d", *(uint32_t *)p_max_value);
		cbor_print("\r\n");
		FAIL();
	}
	cbor_print("\r\n");
	return true;
}


bool uintx32_decode(cbor_state_t * p_state,
		uint32_t *p_result, void *p_min_value, void *p_max_value)
{
	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);

	if (major_type != CBOR_MAJOR_TYPE_PINT) {
		/* Value to be read doesn't have the right type. */
		FAIL();
	}
	if (!uint32_decode(p_state, p_result, p_min_value, p_max_value)){
		FAIL();
	}
	return true;
}


static bool size_decode(cbor_state_t * p_state,
		size_t *p_result, size_t *p_min_value, size_t *p_max_value)
{
	_Static_assert((sizeof(size_t) == sizeof(uint32_t)),
			"This code needs size_t to be 4 bytes long.");
	return uint32_decode(p_state,
			p_result, p_min_value, p_max_value);
}


static bool strx_start_decode(cbor_state_t * p_state,
		cbor_string_type_t *p_result, void *p_min_len, void *p_max_len,
		cbor_major_type_t exp_major_type)
{
	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);
	cbor_string_type_t *p_str_result = (cbor_string_type_t *)p_result;

	if (major_type != exp_major_type) {
		/* Value to be read doesn't have the right type. */
		FAIL();
	}
	if (!size_decode(p_state,
			&p_str_result->len, (size_t *)p_min_len,
			(size_t *)p_max_len)) {
		FAIL();
	}
	p_str_result->value = p_state->p_payload;
	return true;
}

bool bstrx_start_decode(cbor_state_t * p_state,
		cbor_string_type_t *p_result, void *p_min_len, void *p_max_len)
{
	return strx_start_decode(p_state, p_result, p_min_len, p_max_len,
			CBOR_MAJOR_TYPE_BSTR);
}


bool tstrx_start_decode(cbor_state_t * p_state,
		cbor_string_type_t *p_result, void *p_min_len, void *p_max_len)
{
	return strx_start_decode(p_state, p_result, p_min_len, p_max_len,
			CBOR_MAJOR_TYPE_TSTR);
}


bool bstrx_decode(cbor_state_t * p_state,
		cbor_string_type_t *p_result, void *p_min_len, void *p_max_len)
{
	if (!bstrx_start_decode(p_state, p_result,
				p_min_len, p_max_len)) {
		return false;
	}
	(p_state->p_payload) += p_result->len;
	return true;
}

bool tstrx_decode(cbor_state_t * p_state,
		cbor_string_type_t *p_result, void *p_min_len, void *p_max_len)
{
	if (!tstrx_start_decode(p_state, p_result,
				p_min_len, p_max_len)) {
		return false;
	}
	(p_state->p_payload) += p_result->len;
	return true;
}


static bool list_map_start_decode(cbor_state_t *p_state, size_t min_num,
		size_t max_num)
{
	size_t new_elem_count;

	if (!uint32_decode(p_state, &new_elem_count, &min_num,
			&max_num)) {
		FAIL();
	}

	if (!new_backup(p_state, new_elem_count)) {
		FAIL();
	}

	return true;
}


bool list_start_decode(cbor_state_t *p_state, size_t min_num, size_t max_num)
{
	return list_map_start_decode(p_state, min_num, max_num);
}


bool map_start_decode(cbor_state_t *p_state, size_t min_num, size_t max_num)
{
	bool ret = list_map_start_decode(p_state, min_num, max_num);
	if (ret) {
		p_state->elem_count *= 2;
	}
	return ret;
}


bool list_map_end_decode(cbor_state_t *p_state, size_t min_num, size_t max_num)
{
	if (!restore_backup(p_state,
			FLAG_RESTORE | FLAG_DISCARD | FLAG_TRANSFER_PAYLOAD,
			0)) {
		FAIL();
	}

	return true;
}


bool list_end_decode(cbor_state_t *p_state, size_t min_num, size_t max_num)
{
	return list_map_end_decode(p_state, min_num, max_num);
}


bool map_end_decode(cbor_state_t *p_state, size_t min_num, size_t max_num)
{
	return list_map_end_decode(p_state, min_num, max_num);
}


static bool primx_decode(cbor_state_t * p_state,
		uint8_t *p_result, uint32_t *p_min_result, uint32_t *p_max_result)
{
	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);
	uint32_t val;
	uint32_t max_byte = 0xFF;

	if (major_type != CBOR_MAJOR_TYPE_PRIM) {
		/* Value to be read doesn't have the right type. */
		FAIL();
	}
	if (!uint32_decode(p_state, &val, p_min_result,
			p_max_result ? p_max_result : &max_byte)) {
		FAIL();
	}
	if (p_result != NULL) {
		*p_result = val;
	}
	return true;
}


bool nilx_decode(cbor_state_t *p_state,
		uint8_t *p_result, void *p_min_result, void *p_max_result)
{
	uint32_t val = 22;
	if (!primx_decode(p_state, NULL, &val, &val)) {
		FAIL();
	}
	return true;
}


bool boolx_decode(cbor_state_t * p_state,
		bool *p_result, uint32_t *p_min_result, uint32_t *p_max_result)
{
	uint32_t min_result = (*(uint8_t *)p_min_result) + BOOL_TO_PRIM;
	uint32_t max_result = (*(uint8_t *)p_max_result) + BOOL_TO_PRIM;

	cbor_print("min: %d, max: %d\r\n", min_result, max_result);

	if (!primx_decode(p_state,
			(uint8_t *)p_result, &min_result, &max_result)) {
		FAIL();
	}
	cbor_print("val: %u\r\n", *p_result);
	(*(uint8_t *)p_result) -= BOOL_TO_PRIM;
	cbor_print("boolval: %u\r\n", *p_result);
	return true;
}


bool double_decode(cbor_state_t * p_state,
		double *p_result, void *p_min_result, void *p_max_result)
{
	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);

	if (major_type != CBOR_MAJOR_TYPE_PRIM) {
		/* Value to be read doesn't have the right type. */
		FAIL();
	}
	if (!value_extract(p_state, p_result,
			sizeof(*p_result))) {
		FAIL();
	}

	if (!PTR_VALUE_IN_RANGE(double, p_result, p_min_result, p_max_result)) {
		FAIL();
	}
	return true;
}


bool any_decode(cbor_state_t * p_state,
		void *p_result, void *p_min_result, void *p_max_result)
{
	cbor_assert(p_result == NULL,
			"'any' type cannot be returned, only skipped.\n");

	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);
	uint32_t value;
	size_t num_decode;
	void *p_null_result = NULL;
	size_t temp_elem_count;

	if (!value_extract(p_state, &value, sizeof(value))) {
		/* Can happen because of p_elem_count (or p_payload_end) */
		FAIL();
	}

	switch (major_type) {
		case CBOR_MAJOR_TYPE_BSTR:
		case CBOR_MAJOR_TYPE_TSTR:
			(p_state->p_payload) += value;
			break;
		case CBOR_MAJOR_TYPE_MAP:
			value *= 2; /* Because all members have a key. */
			/* Fallthrough */
		case CBOR_MAJOR_TYPE_LIST:
			temp_elem_count = p_state->elem_count;
			p_state->elem_count = value;
			if (!multi_decode(value, value, &num_decode, any_decode,
					p_state,
					&p_null_result,	NULL, NULL, 0)) {
				p_state->elem_count = temp_elem_count;
				FAIL();
			}
			p_state->elem_count = temp_elem_count;
			break;
		default:
			/* Do nothing */
			break;
	}

	return true;
}


bool multi_decode(size_t min_decode,
		size_t max_decode,
		size_t *p_num_decode,
		processor_t decoder,
		cbor_state_t * p_state,
		void *p_result,
		void *p_min_result,
		void *p_max_result,
		size_t result_len)
{
	for (size_t i = 0; i < max_decode; i++) {
		uint8_t const *p_payload_bak = p_state->p_payload;
		size_t elem_count_bak = p_state->elem_count;

		if (!decoder(p_state,
				(uint8_t *)p_result + i*result_len,
				p_min_result,
				p_max_result)) {
			*p_num_decode = i;
			p_state->p_payload = p_payload_bak;
			p_state->elem_count = elem_count_bak;
			if (i < min_decode) {
				FAIL();
			} else {
				cbor_print("Found %zu elements.\n", i);
			}
			return true;
		}
	}
	cbor_print("Found %zu elements.\n", max_decode);
	*p_num_decode = max_decode;
	return true;
}
