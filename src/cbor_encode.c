/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "cbor_encode.h"
#include "cbor_common.h"


uint8_t get_additional(size_t len, uint8_t value0)
{
	switch(len) {
		case 0: return value0;
		case 1: return 24;
		case 2: return 25;
		case 3: return 25;
		case 4: return 26;
		case 5: return 26;
		case 6: return 26;
		case 7: return 26;
		case 8: return 27;
	}

	cbor_assert(false, NULL);
	return 0;
}

static bool encode_header_byte(cbor_state_t *p_state,
	cbor_major_type_t major_type, uint8_t additional)
{
	if ((p_state->p_payload + 1) > p_state->p_payload_end) {
		FAIL();
	}

	cbor_assert(additional < 32, NULL);

	*(p_state->p_payload_mut++) = (major_type << 5) | (additional & 0x1F);
	return true;
}

/** Encode a single value.
 */
static bool value_encode_len(cbor_state_t *p_state, cbor_major_type_t major_type,
		void * const p_result, size_t result_len)
{
	cbor_trace();

	if (p_state->elem_count == 0) {
		FAIL();
	}

	uint8_t *p_u8_result  = (uint8_t *)p_result;

	if ((p_state->p_payload + 1 + result_len) > p_state->p_payload_end) {
		FAIL();
	}

	if (!encode_header_byte(p_state, major_type,
				get_additional(result_len, p_u8_result[0]))) {
		FAIL();
	}

#ifdef CONFIG_BIG_ENDIAN
	memcpy(p_state->p_payload_mut, p_u8_result, result_len);
	p_state->p_payload_mut += result_len;
#else
	for (; result_len > 0; result_len--) {
		*(p_state->p_payload_mut++) = p_u8_result[result_len - 1];
	}
#endif

	p_state->elem_count--;
	return true;
}


static size_t get_result_len(void * const p_result, size_t max_result_len)
{
	uint8_t *p_u8_result  = (uint8_t *)p_result;

	for (; max_result_len > 0; max_result_len--) {
		if (p_u8_result[max_result_len - 1] != 0) {
			break;
		}
	}
	if ((max_result_len == 1) && (p_u8_result[0] <= VALUE_IN_HEADER)) {
		max_result_len = 0;
	}

	return max_result_len;
}


static bool value_encode(cbor_state_t *p_state, cbor_major_type_t major_type,
		void * const p_result, size_t max_result_len)
{
	cbor_assert(max_result_len != 0, "0-length result not supported.\n");
	return value_encode_len(p_state, major_type, p_result,
				get_result_len(p_result, max_result_len));
}


bool intx32_encode(cbor_state_t *p_state,
		int32_t *p_result, void *p_min_value, void *p_max_value)
{
	cbor_major_type_t major_type;
	uint32_t uint_result;

	if (*p_result < 0) {
		major_type = CBOR_MAJOR_TYPE_NINT;
		// Convert from CBOR's representation.
		uint_result = -1 - *p_result;
	} else {
		major_type = CBOR_MAJOR_TYPE_PINT;
		uint_result = *p_result;
	}

	if (!PTR_VALUE_IN_RANGE(int32_t, p_result, p_min_value, p_max_value)) {
		FAIL();
	}

	if (!value_encode(p_state, major_type, &uint_result, 4)) {
		FAIL();
	}

	return true;
}


static bool uint32_encode(cbor_state_t *p_state, cbor_major_type_t major_type,
		void *p_result, void *p_min_value, void *p_max_value)
{
	if (!PTR_VALUE_IN_RANGE(uint32_t, p_result, p_min_value, p_max_value)) {
		if (p_min_value) cbor_print("min: %d ", *(uint32_t *)p_min_value);
		if (p_max_value) cbor_print("max: %d", *(uint32_t *)p_max_value);
		cbor_print("\r\n");
		FAIL();
	}
	cbor_print("val: %u\r\n", *(uint32_t *)p_result);

	if (!value_encode(p_state, major_type, p_result, 4)) {
		FAIL();
	}
	return true;
}


static bool uint_encode(cbor_state_t *p_state, cbor_major_type_t major_type,
		void *p_result, size_t result_len)
{
	cbor_print("val: %u\r\n", *(uint32_t *)p_result);

	if (!value_encode_len(p_state, major_type, p_result, result_len)) {
		FAIL();
	}
	return true;
}


bool uintx32_encode(cbor_state_t *p_state,
		uint32_t *p_result, void *p_min_value, void *p_max_value)
{
	if (!uint32_encode(p_state, CBOR_MAJOR_TYPE_PINT, p_result, p_min_value,
			p_max_value)){
		FAIL();
	}
	return true;
}


static bool strx_start_encode(cbor_state_t *p_state,
		cbor_string_type_t *p_result, size_t *p_min_len,
		size_t *p_max_len, cbor_major_type_t major_type)
{
	if ((get_result_len(&p_result->len, sizeof(p_result->len))
			+ 1 + p_result->len + (uint32_t)p_state->p_payload)
			> (uint32_t)p_state->p_payload_end) {
		FAIL();
	}
	_Static_assert((sizeof(size_t) == sizeof(uint32_t)),
			"This code needs size_t to be 4 bytes long.");
	if (!uint32_encode(p_state, major_type,
			&p_result->len, p_min_len, p_max_len)) {
		FAIL();
	}

	return true;
}


bool bstrx_start_encode(cbor_state_t *p_state,
		cbor_string_type_t *p_result, size_t *p_min_len,
		size_t *p_max_len)
{
	return strx_start_encode(p_state, p_result, p_min_len, p_max_len,
				CBOR_MAJOR_TYPE_BSTR);
}


bool tstrx_start_encode(cbor_state_t *p_state,
		cbor_string_type_t *p_result, size_t *p_min_len,
		size_t *p_max_len)
{
	return strx_start_encode(p_state, p_result, p_min_len, p_max_len,
				CBOR_MAJOR_TYPE_TSTR);
}


static bool strx_encode(cbor_state_t *p_state,
		cbor_string_type_t *p_result, size_t *p_min_len,
		size_t *p_max_len, cbor_major_type_t major_type)
{
	if (!strx_start_encode(p_state, p_result,
				p_min_len, p_max_len, major_type)) {
		FAIL();
	}
	memcpy(p_state->p_payload_mut, p_result->value, p_result->len);
	p_state->p_payload += p_result->len;
	return true;
}


bool bstrx_encode(cbor_state_t *p_state,
		cbor_string_type_t *p_result, size_t *p_min_len,
		size_t *p_max_len)
{
	return strx_encode(p_state, p_result, p_min_len, p_max_len,
				CBOR_MAJOR_TYPE_BSTR);
}


bool tstrx_encode(cbor_state_t *p_state,
		cbor_string_type_t *p_result, size_t *p_min_len,
		size_t *p_max_len)
{
	return strx_encode(p_state, p_result, p_min_len, p_max_len,
				CBOR_MAJOR_TYPE_TSTR);
}


static bool list_map_start_encode(cbor_state_t *p_state, size_t min_num,
		size_t max_num, cbor_major_type_t major_type)
{
	if (!new_backup(p_state, 1 /* add 1 because of below uint32_encode */
				 + ((major_type == CBOR_MAJOR_TYPE_LIST)
					? max_num : max_num * 2))) {
		FAIL();
	}

	/* Encode header with max number of elements, as a placeholder. */
	if (!uint32_encode(p_state, major_type, &max_num, &min_num, &max_num)) {
		FAIL();
	}
	return true;
}


bool list_start_encode(cbor_state_t *p_state, size_t min_num, size_t max_num)
{
	return list_map_start_encode(p_state, min_num, max_num,
					CBOR_MAJOR_TYPE_LIST);
}


bool map_start_encode(cbor_state_t *p_state, size_t min_num, size_t max_num)
{
	return list_map_start_encode(p_state, min_num, max_num,
					CBOR_MAJOR_TYPE_MAP);
}


bool list_map_end_encode(cbor_state_t *p_state, size_t min_num, size_t max_num,
			cbor_major_type_t major_type)
{
	size_t list_count = max_num - ((major_type == CBOR_MAJOR_TYPE_LIST) ?
					p_state->elem_count
					: (p_state->elem_count / 2));
	size_t max_elem_count = ((major_type == CBOR_MAJOR_TYPE_LIST) ?
					(max_num - min_num)
					: ((max_num - min_num) * 2));

	const uint8_t *p_payload = p_state->p_payload;

	if (!restore_backup(p_state, FLAG_RESTORE | FLAG_DISCARD,
			max_elem_count)) {
		FAIL();
	}

	cbor_print("list_count: %d\r\n", list_count);

	/* Reencode header of list now that we know the number of elements. */
	if (!(uint_encode(p_state, major_type, &list_count,
			get_result_len(&max_num, 4)))) {
		FAIL();
	}

	/* Reset payload pointer to end of list */
	p_state->p_payload = p_payload;

	return true;
}


bool list_end_encode(cbor_state_t *p_state, size_t min_num, size_t max_num)
{
	return list_map_end_encode(p_state, min_num, max_num,
					CBOR_MAJOR_TYPE_LIST);
}


bool map_end_encode(cbor_state_t *p_state, size_t min_num, size_t max_num)
{
	return list_map_end_encode(p_state, min_num, max_num,
					CBOR_MAJOR_TYPE_MAP);
}


static bool primx_encode(cbor_state_t *p_state,
		uint8_t *p_result, uint32_t *p_min_result, uint32_t *p_max_result)
{
	uint32_t val;

	if (p_result != NULL) {
		val = *p_result;
	}

	if (!uint32_encode(p_state, CBOR_MAJOR_TYPE_PRIM,
			&val, p_min_result, p_max_result)) {
		FAIL();
	}
	return true;
}

bool nilx_encode(cbor_state_t *p_state,
		uint8_t *p_result, void *p_min_result, void *p_max_result)
{
	uint8_t val = 22;
	uint32_t minmax = 22;
	return primx_encode(p_state, &val, &minmax, &minmax);
}

bool boolx_encode(cbor_state_t *p_state,
		bool *p_result, uint32_t *p_min_result, uint32_t *p_max_result)
{
	uint32_t min_result = *p_min_result + BOOL_TO_PRIM;
	uint32_t max_result = *p_max_result + BOOL_TO_PRIM;

	(*p_result) += BOOL_TO_PRIM;

	if (!primx_encode(p_state,
			(uint8_t *)p_result, &min_result, &max_result)) {
		FAIL();
	}
	return true;
}


bool double_encode(cbor_state_t *p_state,
		double *p_result, double *p_min_result, double *p_max_result)
{
	if (!PTR_VALUE_IN_RANGE(double, p_result, p_min_result, p_max_result)) {
		FAIL();
	}

	if (!value_encode(p_state, CBOR_MAJOR_TYPE_PRIM, p_result,
			sizeof(*p_result))) {
		FAIL();
	}

	return true;
}


bool multi_encode(size_t min_encode,
		size_t max_encode,
		size_t *p_num_encode,
		processor_t encoder,
		cbor_state_t *p_state,
		void *p_result,
		void *p_min_result,
		void *p_max_result,
		size_t result_len)
{
	if (!PTR_VALUE_IN_RANGE(size_t, p_num_encode, &min_encode, &max_encode)) {
		FAIL();
	}
	for (size_t i = 0; i < *p_num_encode; i++) {
		uint8_t const *p_payload_bak = p_state->p_payload;
		size_t elem_count_bak = p_state->elem_count;

		if (!encoder(p_state,
				(uint8_t *)p_result + i*result_len,
				p_min_result,
				p_max_result)) {
			p_state->p_payload = p_payload_bak;
			p_state->elem_count = elem_count_bak;
			FAIL();
		}
	}
	cbor_print("Found %zu elements.\n", *p_num_encode);
	return true;
}
