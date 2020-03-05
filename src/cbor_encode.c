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

	*(p_state->p_payload_mut) = (major_type << 5) | (additional & 0x1F);
	return true;
}

/** Encode a single value.
 */
static bool value_encode_len(cbor_state_t *p_state, cbor_major_type_t major_type,
		const void * const p_input, size_t result_len)
{
	uint8_t *p_u8_result  = (uint8_t *)p_input;

	if ((p_state->p_payload + 1 + result_len) > p_state->p_payload_end) {
		FAIL();
	}

	if (!encode_header_byte(p_state, major_type,
				get_additional(result_len, p_u8_result[0]))) {
		FAIL();
	}
	cbor_trace();
	p_state->p_payload_mut++;

#ifdef CONFIG_BIG_ENDIAN
	memcpy(p_state->p_payload_mut, p_u8_result, result_len);
	p_state->p_payload_mut += result_len;
#else
	for (; result_len > 0; result_len--) {
		*(p_state->p_payload_mut++) = p_u8_result[result_len - 1];
	}
#endif

	p_state->elem_count++;
	return true;
}


static size_t get_result_len(const void * const p_input, size_t max_result_len)
{
	uint8_t *p_u8_result  = (uint8_t *)p_input;

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
		const void * const p_input, size_t max_result_len)
{
	cbor_assert(max_result_len != 0, "0-length result not supported.\n");
	return value_encode_len(p_state, major_type, p_input,
				get_result_len(p_input, max_result_len));
}


bool intx32_encode(cbor_state_t *p_state, const int32_t *p_input)
{
	cbor_major_type_t major_type;
	uint32_t uint_input;

	if (*p_input < 0) {
		major_type = CBOR_MAJOR_TYPE_NINT;
		// Convert from CBOR's representation.
		uint_input = -1 - *p_input;
	} else {
		major_type = CBOR_MAJOR_TYPE_PINT;
		uint_input = *p_input;
	}

	if (!value_encode(p_state, major_type, &uint_input, 4)) {
		FAIL();
	}

	return true;
}


static bool uint32_encode(cbor_state_t *p_state, const uint32_t *p_input,
		cbor_major_type_t major_type)
{
	if (!value_encode(p_state, major_type, p_input, 4)) {
		FAIL();
	}
	return true;
}


static bool uint_encode(cbor_state_t *p_state, cbor_major_type_t major_type,
		void *p_input, size_t result_len)
{
	cbor_print("val: %u\r\n", *(const uint32_t *)p_input);

	if (!value_encode_len(p_state, major_type, p_input, result_len)) {
		FAIL();
	}
	return true;
}


bool uintx32_encode(cbor_state_t *p_state, const uint32_t *p_input)
{
	if (!uint32_encode(p_state, p_input, CBOR_MAJOR_TYPE_PINT)){
		FAIL();
	}
	return true;
}


static bool strx_start_encode(cbor_state_t *p_state,
		const cbor_string_type_t *p_input, cbor_major_type_t major_type)
{
	if (p_input->value && ((get_result_len(&p_input->len, sizeof(p_input->len))
			+ 1 + p_input->len + (uint32_t)p_state->p_payload)
			> (uint32_t)p_state->p_payload_end)) {
		FAIL();
	}
	_Static_assert((sizeof(size_t) == sizeof(uint32_t)),
			"This code needs size_t to be 4 bytes long.");
	if (!uint32_encode(p_state, &p_input->len, major_type)) {
		FAIL();
	}

	return true;
}


static size_t remaining_str_len(cbor_state_t *p_state)
{
	size_t max_len = (size_t)p_state->p_payload_end - (size_t)p_state->p_payload;
	size_t result_len = get_result_len(&max_len, sizeof(size_t));
	return max_len - result_len - 1;
}


bool bstrx_cbor_start_encode(cbor_state_t *p_state)
{
	if (!new_backup(p_state, 0)) {
		FAIL();
	}

	size_t max_len = remaining_str_len(p_state);

	/* Encode a dummy header */
	if (!uint32_encode(p_state, &max_len,
			CBOR_MAJOR_TYPE_BSTR)) {
		FAIL();
	}
	return true;
}

bool bstrx_cbor_end_encode(cbor_state_t *p_state)
{
	const uint8_t *p_payload = p_state->p_payload;

	if (!restore_backup(p_state, FLAG_RESTORE | FLAG_DISCARD, 0xFFFFFFFF)) {
		FAIL();
	}
	cbor_string_type_t value;

	value.value = p_state->p_payload_end - remaining_str_len(p_state);
	value.len = (size_t)p_payload - (size_t)value.value;

	/* Reencode header of list now that we know the number of elements. */
	if (!bstrx_encode(p_state, &value)) {
		FAIL();
	}

	return true;
}


static bool strx_encode(cbor_state_t *p_state,
		const cbor_string_type_t *p_input, cbor_major_type_t major_type)
{
	if (!strx_start_encode(p_state, p_input, major_type)) {
		FAIL();
	}
	if (p_state->p_payload_mut != p_input->value) {
		memmove(p_state->p_payload_mut, p_input->value, p_input->len);
	}
	p_state->p_payload += p_input->len;
	return true;
}


bool bstrx_encode(cbor_state_t *p_state, const cbor_string_type_t *p_input)
{
	return strx_encode(p_state, p_input, CBOR_MAJOR_TYPE_BSTR);
}


bool tstrx_encode(cbor_state_t *p_state, const cbor_string_type_t *p_input)
{
	return strx_encode(p_state, p_input, CBOR_MAJOR_TYPE_TSTR);
}


static bool list_map_start_encode(cbor_state_t *p_state, size_t max_num,
		cbor_major_type_t major_type)
{
	if (!new_backup(p_state, 0)) {
		FAIL();
	}

	/* Encode dummy header with max number of elements. */
	if (!uint32_encode(p_state, &max_num, major_type)) {
		FAIL();
	}
	p_state->elem_count--; /* Because of dummy header. */
	return true;
}


bool list_start_encode(cbor_state_t *p_state, size_t max_num)
{
	return list_map_start_encode(p_state, max_num, CBOR_MAJOR_TYPE_LIST);
}


bool map_start_encode(cbor_state_t *p_state, size_t max_num)
{
	return list_map_start_encode(p_state, max_num, CBOR_MAJOR_TYPE_MAP);
}


bool list_map_end_encode(cbor_state_t *p_state, size_t max_num,
			cbor_major_type_t major_type)
{
	size_t list_count = ((major_type == CBOR_MAJOR_TYPE_LIST) ?
					p_state->elem_count
					: (p_state->elem_count / 2));

	const uint8_t *p_payload = p_state->p_payload;

	if (!restore_backup(p_state, FLAG_RESTORE | FLAG_DISCARD, 0xFFFFFFFF)) {
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


bool list_end_encode(cbor_state_t *p_state, size_t max_num)
{
	return list_map_end_encode(p_state, max_num, CBOR_MAJOR_TYPE_LIST);
}


bool map_end_encode(cbor_state_t *p_state, size_t max_num)
{
	return list_map_end_encode(p_state, max_num, CBOR_MAJOR_TYPE_MAP);
}


static bool primx_encode(cbor_state_t *p_state, uint32_t input)
{
	if (!uint32_encode(p_state, &input, CBOR_MAJOR_TYPE_PRIM)) {
		FAIL();
	}
	return true;
}


bool nilx_encode(cbor_state_t *p_state, const void *p_input)
{
	(void)p_input;
	return primx_encode(p_state, 22);
}


bool boolx_encode(cbor_state_t *p_state, const bool *p_input)
{
	if (!primx_encode(p_state, *p_input + BOOL_TO_PRIM)) {
		FAIL();
	}
	return true;
}


bool double_encode(cbor_state_t *p_state, double *p_input)
{
	if (!value_encode(p_state, CBOR_MAJOR_TYPE_PRIM, p_input,
			sizeof(*p_input))) {
		FAIL();
	}

	return true;
}


bool any_encode(cbor_state_t *p_state, void *p_input)
{
	return nilx_encode(p_state, p_input);
}


bool multi_encode(size_t min_encode,
		size_t max_encode,
		const size_t *p_num_encode,
		cbor_encoder_t encoder,
		cbor_state_t *p_state,
		const void *p_input,
		size_t result_len)
{
	if (!PTR_VALUE_IN_RANGE(size_t, p_num_encode, NULL, &max_encode)) {
		FAIL();
	}
	for (size_t i = 0; i < *p_num_encode; i++) {
		uint8_t const *p_payload_bak = p_state->p_payload;
		size_t elem_count_bak = p_state->elem_count;

		if (!encoder(p_state, (const uint8_t *)p_input + i*result_len)) {
			p_state->p_payload = p_payload_bak;
			p_state->elem_count = elem_count_bak;
			FAIL();
		}
	}
	cbor_print("Found %zu elements.\n", *p_num_encode);
	return true;
}


bool present_encode(const size_t *p_present,
		cbor_encoder_t encoder,
		cbor_state_t * p_state,
		const void *p_input)
{
	size_t num_encode = *p_present;
	bool retval = multi_encode(0, 1, &num_encode, encoder, p_state, p_input, 0);
	return retval;
}
