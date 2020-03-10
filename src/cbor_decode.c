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


static bool int32_decode(cbor_state_t * p_state, int32_t *p_result)
{
	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);
	uint32_t uint_result;
	int32_t int_result;

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
		int_result = -1 - uint_result;
	} else {
		int_result = uint_result;
	}

	cbor_print("val: %d\r\n", int_result);
	*p_result = int_result;
	return true;
}


bool intx32_decode(cbor_state_t * p_state, int32_t *p_result)
{
	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);

	if (major_type != CBOR_MAJOR_TYPE_PINT
		&& major_type != CBOR_MAJOR_TYPE_NINT) {
		/* Value to be read doesn't have the right type. */
		FAIL();
	}

	if (!int32_decode(p_state, p_result)){
		FAIL();
	}
	return true;
}

bool intx32_expect(cbor_state_t * p_state, int32_t *p_result)
{
	int32_t value;
	if (!intx32_decode(p_state, &value)) {
		FAIL();
	}
	if (value != *p_result) {
		cbor_print("%d != %d\r\n", value, *p_result);
		FAIL();
	}
	return true;
}


static bool uint32_decode(cbor_state_t * p_state, uint32_t *p_result)
{
	if (!value_extract(p_state, p_result, 4)) {
		FAIL();
	}

	return true;
}


bool uintx32_decode(cbor_state_t * p_state, uint32_t *p_result)
{
	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);

	if (major_type != CBOR_MAJOR_TYPE_PINT) {
		/* Value to be read doesn't have the right type. */
		FAIL();
	}
	if (!uint32_decode(p_state, p_result)){
		FAIL();
	}
	return true;
}

bool uintx32_expect(cbor_state_t * p_state, uint32_t *p_result)
{
	uint32_t value;
	if (!uintx32_decode(p_state, &value)) {
		FAIL();
	}
	if (value != *p_result) {
		cbor_print("%u != %u\r\n", value, *p_result);
		FAIL();
	}
	return true;
}


static bool strx_start_decode(cbor_state_t * p_state,
		cbor_string_type_t *p_result, cbor_major_type_t exp_major_type)
{
	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);

	if (major_type != exp_major_type) {
		FAIL();
	}

	_Static_assert((sizeof(size_t) == sizeof(uint32_t)),
			"This code needs size_t to be 4 bytes long.");
	if (!uint32_decode(p_state, &p_result->len)) {
		FAIL();
	}

	if ((p_state->p_payload + p_result->len) > p_state->p_payload_end) {
		cbor_print("error: 0x%x > 0x%x\r\n",
		(uint32_t)(p_state->p_payload + p_result->len),
		(uint32_t)p_state->p_payload_end);
		FAIL();
	}

	p_result->value = p_state->p_payload;
	return true;
}

bool bstrx_cbor_start_decode(cbor_state_t *p_state)
{
	cbor_string_type_t value;

	if(!strx_start_decode(p_state, &value, CBOR_MAJOR_TYPE_BSTR)) {
		FAIL();
	}

	if (!new_backup(p_state, 0xFFFFFFFF)) {
		FAIL();
	}

	p_state->p_payload_end = value.value + value.len;
	return true;
}

bool bstrx_cbor_end_decode(cbor_state_t *p_state)
{
	if (p_state->p_payload != p_state->p_payload_end) {
		FAIL();
	}
	if (!restore_backup(p_state,
			FLAG_RESTORE | FLAG_DISCARD | FLAG_TRANSFER_PAYLOAD,
			0xFFFFFFFF)) {
		FAIL();
	}

	return true;
}


bool strx_decode(cbor_state_t * p_state, cbor_string_type_t *p_result,
		cbor_major_type_t exp_major_type)
{
	if (!strx_start_decode(p_state, p_result, exp_major_type)) {
		FAIL();
	}

	(p_state->p_payload) += p_result->len;
	return true;
}


bool strx_expect(cbor_state_t *p_state, cbor_string_type_t *p_result,
		cbor_major_type_t exp_major_type)
{
	cbor_string_type_t result;
	if (!strx_decode(p_state, &result, exp_major_type)) {
		FAIL();
	}
	if ((result.len != p_result->len)
			|| memcmp(p_result->value, result.value, result.len)) {
		FAIL();
	}
	return true;
}


bool bstrx_decode(cbor_state_t * p_state, cbor_string_type_t *p_result)
{
	return strx_decode(p_state, p_result, CBOR_MAJOR_TYPE_BSTR);
}


bool bstrx_expect(cbor_state_t * p_state, cbor_string_type_t *p_result)
{
	return strx_expect(p_state, p_result, CBOR_MAJOR_TYPE_BSTR);
}


bool tstrx_decode(cbor_state_t * p_state, cbor_string_type_t *p_result)
{
	return strx_decode(p_state, p_result, CBOR_MAJOR_TYPE_TSTR);
}


bool tstrx_expect(cbor_state_t * p_state, cbor_string_type_t *p_result)
{
	return strx_expect(p_state, p_result, CBOR_MAJOR_TYPE_TSTR);
}


static bool list_map_start_decode(cbor_state_t *p_state,
		cbor_major_type_t exp_major_type)
{
	size_t new_elem_count;
	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);

	if (major_type != exp_major_type) {
		FAIL();
	}

	if (!uint32_decode(p_state, &new_elem_count)) {
		FAIL();
	}

	if (!new_backup(p_state, new_elem_count)) {
		FAIL();
	}

	return true;
}


bool list_start_decode(cbor_state_t *p_state)
{
	return list_map_start_decode(p_state, CBOR_MAJOR_TYPE_LIST);
}


bool map_start_decode(cbor_state_t *p_state)
{
	bool ret = list_map_start_decode(p_state, CBOR_MAJOR_TYPE_MAP);
	if (ret) {
		p_state->elem_count *= 2;
	}
	return ret;
}


bool list_map_end_decode(cbor_state_t *p_state)
{
	if (!restore_backup(p_state,
			FLAG_RESTORE | FLAG_DISCARD | FLAG_TRANSFER_PAYLOAD,
			0)) {
		FAIL();
	}

	return true;
}


bool list_end_decode(cbor_state_t *p_state)
{
	return list_map_end_decode(p_state);
}


bool map_end_decode(cbor_state_t *p_state)
{
	return list_map_end_decode(p_state);
}


static bool primx_decode(cbor_state_t * p_state, uint32_t *p_result)
{
	uint8_t major_type = MAJOR_TYPE(*p_state->p_payload);

	if (major_type != CBOR_MAJOR_TYPE_PRIM) {
		/* Value to be read doesn't have the right type. */
		FAIL();
	}
	if (!uint32_decode(p_state, p_result)) {
		FAIL();
	}
	if (*p_result > 0xFF) {
		FAIL();
	}
	return true;
}

static bool primx_expect(cbor_state_t * p_state, uint32_t result)
{
	uint32_t value;
	if (!primx_decode(p_state, &value)){
		FAIL();
	}
	if (value != result) {
		FAIL();
	}
	return true;
}


bool nilx_expect(cbor_state_t *p_state, void *p_result)
{
	if (!primx_expect(p_state, 22)) {
		FAIL();
	}
	return true;
}


bool boolx_decode(cbor_state_t * p_state, bool *p_result)
{
	uint32_t result;

	if (!primx_decode(p_state, &result)) {
		FAIL();
	}
	(*p_result) = result - BOOL_TO_PRIM;

	cbor_print("boolval: %u\r\n", *p_result);
	return true;
}


bool boolx_expect(cbor_state_t * p_state, bool *p_result)
{
	bool value;
	if (!boolx_decode(p_state, &value)) {
		FAIL();
	}
	if (value != *p_result) {
		FAIL();
	}
	return true;
}


bool double_decode(cbor_state_t * p_state, double *p_result)
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
	return true;
}


bool double_expect(cbor_state_t * p_state, double *p_result)
{
	double value;
	if (!double_decode(p_state, &value)) {
		FAIL();
	}
	if (value != *p_result) {
		FAIL();
	}
	return true;
}


bool any_decode(cbor_state_t * p_state, void *p_result)
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
			if (!multi_decode(value, value, &num_decode,
					(void *)any_decode, p_state,
					&p_null_result,	0)) {
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
		cbor_decoder_t decoder,
		cbor_state_t * p_state,
		void *p_result,
		size_t result_len)
{
	for (size_t i = 0; i < max_decode; i++) {
		uint8_t const *p_payload_bak = p_state->p_payload;
		size_t elem_count_bak = p_state->elem_count;

		if (!decoder(p_state,
				(uint8_t *)p_result + i*result_len)) {
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


bool present_decode(size_t *p_present,
		cbor_decoder_t decoder,
		cbor_state_t * p_state,
		void *p_result)
{
	size_t num_decode;
	bool retval = multi_decode(0, 1, &num_decode, decoder, p_state, p_result, 0);
	if (retval) {
		*p_present = num_decode;
	}
	return retval;
}
