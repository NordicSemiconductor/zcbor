/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_decode.h"
#include "zcbor_common.h"


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

/** The value where the INDET_LEN bit (bit 31) is set, and elem_count is 0.
 *  This represents the minimum value elem_count can have for indeterminate
 *  length arrays. It also functions as a mask for the INDET_LEN bit.
 */
#define MIN_INDET_LEN_ELEM_COUNT 0x80000000

/** Check the most significant bit to see if we are processing an indeterminate
 *  length array.
 */
#define INDET_LEN(elem_count) (elem_count >= MIN_INDET_LEN_ELEM_COUNT)

/** Initial value for elem_count for indeterminate length arrays. */
#define INDET_LEN_ELEM_COUNT 0xFFFFFFF0


#define FAIL_AND_DECR_IF(expr) \
do {\
	if (expr) { \
		(state->payload)--; \
		ZCBOR_FAIL(); \
	} \
} while(0)

#define FAIL_IF(expr) \
do {\
	if (expr) { \
		ZCBOR_FAIL(); \
	} \
} while(0)


#define FAIL_RESTORE() \
	state->payload = state->payload_bak; \
	state->elem_count++; \
	ZCBOR_FAIL()

/** Get a single value.
 *
 * @details @p ppayload must point to the header byte. This function will
 *          retrieve the value (either from within the additional info, or from
 *          the subsequent bytes) and return it in the result. The result can
 *          have arbitrary length.
 *
 *          The function will also validate
 *           - Min/max constraints on the value.
 *           - That @p payload doesn't overrun past @p payload_end.
 *           - That @p elem_count has not been exhausted.
 *
 *          @p ppayload and @p elem_count are updated if the function
 *          succeeds. If not, they are left unchanged.
 *
 *          CBOR values are always big-endian, so this function converts from
 *          big to little-endian if necessary (@ref CONFIG_BIG_ENDIAN).
 */
static bool value_extract(zcbor_state_t *state,
		void *const result, uint32_t result_len)
{
	zcbor_trace();
	zcbor_assert(result_len != 0, "0-length result not supported.\n");
	zcbor_assert(result != NULL, NULL);

	FAIL_IF((state->elem_count == 0) \
		|| (state->elem_count == MIN_INDET_LEN_ELEM_COUNT) \
		|| (state->payload >= state->payload_end));

	uint8_t *u8_result  = (uint8_t *)result;
	uint8_t additional = ADDITIONAL(*state->payload);

	state->payload_bak = state->payload;
	(state->payload)++;

	memset(result, 0, result_len);
	if (additional <= ZCBOR_VALUE_IN_HEADER) {
#ifdef CONFIG_BIG_ENDIAN
		u8_result[result_len - 1] = additional;
#else
		u8_result[0] = additional;
#endif /* CONFIG_BIG_ENDIAN */
	} else {
		uint32_t len = additional_len(additional);

		FAIL_AND_DECR_IF(len > result_len);
		FAIL_AND_DECR_IF((state->payload + len)
				> state->payload_end);

#ifdef CONFIG_BIG_ENDIAN
		memcpy(&u8_result[result_len - len], state->payload, len);
#else
		for (uint32_t i = 0; i < len; i++) {
			u8_result[i] = (state->payload)[len - i - 1];
		}
#endif /* CONFIG_BIG_ENDIAN */

		(state->payload) += len;
	}

	(state->elem_count)--;
	return true;
}


bool zcbor_int32_decode(zcbor_state_t *state, int32_t *result)
{
	int64_t result64;

	if (zcbor_int64_decode(state, &result64)) {
		if (result64 > INT32_MAX) {
			FAIL_RESTORE();
		}
		*result = result64;
		return true;
	} else {
		ZCBOR_FAIL();
	}
}


bool zcbor_int64_decode(zcbor_state_t *state, int64_t *result)
{
	FAIL_IF(state->payload >= state->payload_end);
	uint8_t major_type = MAJOR_TYPE(*state->payload);
	uint64_t uint_result;
	int64_t int_result;

	if (major_type != ZCBOR_MAJOR_TYPE_PINT
		&& major_type != ZCBOR_MAJOR_TYPE_NINT) {
		/* Value to be read doesn't have the right type. */
		ZCBOR_FAIL();
	}

	if (!value_extract(state, &uint_result, sizeof(uint_result))) {
		ZCBOR_FAIL();
	}

	zcbor_print("uintval: %" PRIu64 "\r\n", uint_result);

	int_result = (int64_t)uint_result;

	if (int_result < 0) {
		/* Value is too large to fit in a signed integer. */
		FAIL_RESTORE();
	}

	if (major_type == ZCBOR_MAJOR_TYPE_NINT) {
		/* Convert from CBOR's representation. */
		*result = -1 - int_result;
	} else {
		*result = int_result;
	}

	zcbor_print("val: %" PRIi64 "\r\n", *result);
	return true;
}


bool zcbor_uint32_decode(zcbor_state_t *state, uint32_t *result)
{
	FAIL_IF(state->payload >= state->payload_end);
	uint8_t major_type = MAJOR_TYPE(*state->payload);

	if (major_type != ZCBOR_MAJOR_TYPE_PINT) {
		/* Value to be read doesn't have the right type. */
		ZCBOR_FAIL();
	}
	if (!value_extract(state, result, sizeof(*result))) {
		ZCBOR_FAIL();
	}
	return true;
}


bool zcbor_uint32_expect_union(zcbor_state_t *state, uint32_t result)
{
	zcbor_union_elem_code(state);
	return zcbor_uint32_expect(state, result);
}


bool zcbor_int32_expect(zcbor_state_t *state, int32_t result)
{
	return zcbor_int64_expect(state, result);
}


bool zcbor_int64_expect(zcbor_state_t *state, int64_t result)
{
	int64_t value;

	if (!zcbor_int64_decode(state, &value)) {
		ZCBOR_FAIL();
	}

	if (value != result) {
		zcbor_print("%" PRIi64 " != %" PRIi64 "\r\n", value, result);
		FAIL_RESTORE();
	}
	return true;
}


bool zcbor_uint64_decode(zcbor_state_t *state, uint64_t *result)
{
	FAIL_IF(state->payload >= state->payload_end);
	uint8_t major_type = MAJOR_TYPE(*state->payload);

	if (major_type != ZCBOR_MAJOR_TYPE_PINT) {
		/* Value to be read doesn't have the right type. */
		ZCBOR_FAIL();
	}
	if (!value_extract(state, result, sizeof(*result))) {
		ZCBOR_FAIL();
	}
	return true;
}


bool zcbor_uint32_expect(zcbor_state_t *state, uint32_t result)
{
	return zcbor_uint64_expect(state, result);
}


bool zcbor_uint64_expect(zcbor_state_t *state, uint64_t result)
{
	uint64_t value;

	if (!zcbor_uint64_decode(state, &value)) {
		ZCBOR_FAIL();
	}
	if (value != result) {
		zcbor_print("%" PRIu64 " != %" PRIu64 "\r\n", value, result);
		FAIL_RESTORE();
	}
	return true;
}

static bool strx_start_decode(zcbor_state_t *state,
		zcbor_string_type_t *result, zcbor_major_type_t exp_major_type)
{
	FAIL_IF(state->payload >= state->payload_end);
	uint8_t major_type = MAJOR_TYPE(*state->payload);

	if (major_type != exp_major_type) {
		ZCBOR_FAIL();
	}

	if (!value_extract(state, &result->len, sizeof(result->len))) {
		ZCBOR_FAIL();
	}

	if (result->len > (state->payload_end - state->payload)) {
		zcbor_print("error: 0x%zu > 0x%zu\r\n",
		result->len,
		(state->payload_end - state->payload));
		FAIL_RESTORE();
	}

	result->value = state->payload;
	return true;
}

bool zcbor_bstr_start_decode(zcbor_state_t *state, zcbor_string_type_t *result)
{
	if(!strx_start_decode(state, result, ZCBOR_MAJOR_TYPE_BSTR)) {
		ZCBOR_FAIL();
	}

	if (!zcbor_new_backup(state, 0xFFFFFFFF)) {
		FAIL_RESTORE();
	}

	/* Overflow is checked in strx_start_decode() */
	state->payload_end = result->value + result->len;
	return true;
}

bool zcbor_bstr_end_decode(zcbor_state_t *state)
{
	if (state->payload != state->payload_end) {
		ZCBOR_FAIL();
	}
	if (!zcbor_process_backup(state,
			ZCBOR_FLAG_RESTORE | ZCBOR_FLAG_CONSUME | ZCBOR_FLAG_TRANSFER_PAYLOAD,
			0xFFFFFFFF)) {
		ZCBOR_FAIL();
	}

	return true;
}


bool strx_decode(zcbor_state_t *state, zcbor_string_type_t *result,
		zcbor_major_type_t exp_major_type)
{
	if (!strx_start_decode(state, result, exp_major_type)) {
		ZCBOR_FAIL();
	}

	/* Overflow is checked in strx_start_decode() */
	(state->payload) += result->len;
	return true;
}


bool strx_expect(zcbor_state_t *state, zcbor_string_type_t *result,
		zcbor_major_type_t exp_major_type)
{
	zcbor_string_type_t tmp_result;

	if (!strx_decode(state, &tmp_result, exp_major_type)) {
		ZCBOR_FAIL();
	}
	if ((tmp_result.len != result->len)
			|| memcmp(result->value, tmp_result.value, tmp_result.len)) {
		FAIL_RESTORE();
	}
	return true;
}


bool zcbor_bstr_decode(zcbor_state_t *state, zcbor_string_type_t *result)
{
	return strx_decode(state, result, ZCBOR_MAJOR_TYPE_BSTR);
}


bool zcbor_bstr_expect(zcbor_state_t *state, zcbor_string_type_t *result)
{
	return strx_expect(state, result, ZCBOR_MAJOR_TYPE_BSTR);
}


bool zcbor_tstr_decode(zcbor_state_t *state, zcbor_string_type_t *result)
{
	return strx_decode(state, result, ZCBOR_MAJOR_TYPE_TSTR);
}


bool zcbor_tstr_expect(zcbor_state_t *state, zcbor_string_type_t *result)
{
	return strx_expect(state, result, ZCBOR_MAJOR_TYPE_TSTR);
}


static bool list_zcbor_map_start_decode(zcbor_state_t *state,
		zcbor_major_type_t exp_major_type)
{
	FAIL_IF(state->payload >= state->payload_end);
	uint8_t major_type = MAJOR_TYPE(*state->payload);
	uint32_t new_elem_count;

	if (major_type != exp_major_type) {
		ZCBOR_FAIL();
	}

	if (ADDITIONAL(*state->payload) == 0x1F) {
		/* Indeterminate length array. */
		new_elem_count = INDET_LEN_ELEM_COUNT;
		state->payload++;
	} else {
		if (!value_extract(state, &new_elem_count, sizeof(new_elem_count))) {
			ZCBOR_FAIL();
		} else if (INDET_LEN(new_elem_count)) {
			/* The new elem_count interferes with the INDET_LEN bit. */
			FAIL_RESTORE();
		}
	}

	if (!zcbor_new_backup(state, new_elem_count)) {
		FAIL_RESTORE();
	}

	return true;
}


bool zcbor_list_start_decode(zcbor_state_t *state)
{
	return list_zcbor_map_start_decode(state, ZCBOR_MAJOR_TYPE_LIST);
}


bool zcbor_map_start_decode(zcbor_state_t *state)
{
	bool ret = list_zcbor_map_start_decode(state, ZCBOR_MAJOR_TYPE_MAP);

	if (ret && !INDET_LEN(state->elem_count)) {
		if (INDET_LEN(state->elem_count * 2)) {
			/* The new elem_count interferes with the INDET_LEN bit. */
			FAIL_RESTORE();
		}
		state->elem_count *= 2;
	}
	return ret;
}


static bool array_end_expect(zcbor_state_t *state)
{
	FAIL_IF(state->payload >= state->payload_end);
	FAIL_IF(*state->payload != 0xFF);

	state->payload++;
	return true;
}


bool list_zcbor_map_end_decode(zcbor_state_t *state)
{
	uint32_t max_elem_count = 0;
	if (INDET_LEN(state->elem_count)) {
		if (!array_end_expect(state)) {
			ZCBOR_FAIL();
		}
		max_elem_count = 0xFFFFFFFF;
	}
	if (!zcbor_process_backup(state,
			ZCBOR_FLAG_RESTORE | ZCBOR_FLAG_CONSUME | ZCBOR_FLAG_TRANSFER_PAYLOAD,
			max_elem_count)) {
		ZCBOR_FAIL();
	}

	return true;
}


bool zcbor_list_end_decode(zcbor_state_t *state)
{
	return list_zcbor_map_end_decode(state);
}


bool zcbor_map_end_decode(zcbor_state_t *state)
{
	return list_zcbor_map_end_decode(state);
}


static bool primx_expect(zcbor_state_t *state, uint8_t result)
{
	FAIL_IF(state->payload >= state->payload_end);
	uint8_t major_type = MAJOR_TYPE(*state->payload);
	uint32_t value;

	if (major_type != ZCBOR_MAJOR_TYPE_PRIM) {
		/* Value to be read doesn't have the right type. */
		ZCBOR_FAIL();
	}

	if (!value_extract(state, &value, sizeof(value))) {
		ZCBOR_FAIL();
	}

	if (value != result) {
		FAIL_RESTORE();
	}
	return true;
}


bool zcbor_nil_expect(zcbor_state_t *state, void *result)
{
	if (!primx_expect(state, 22)) {
		ZCBOR_FAIL();
	}
	return true;
}


bool zcbor_bool_decode(zcbor_state_t *state, bool *result)
{
	if (zcbor_bool_expect(state, false)) {
		*result = false;
	} else if (zcbor_bool_expect(state, true)) {
		*result = true;
	} else {
		ZCBOR_FAIL();
	}

	zcbor_print("boolval: %u\r\n", *result);
	return true;
}


bool zcbor_bool_expect(zcbor_state_t *state, bool result)
{
	if (!primx_expect(state, (uint8_t)(!!result) + ZCBOR_BOOL_TO_PRIM)) {
		ZCBOR_FAIL();
	}
	return true;
}


bool double_decode(zcbor_state_t *state, double *result)
{
	FAIL_IF(state->payload >= state->payload_end);
	uint8_t major_type = MAJOR_TYPE(*state->payload);

	if (major_type != ZCBOR_MAJOR_TYPE_PRIM) {
		/* Value to be read doesn't have the right type. */
		ZCBOR_FAIL();
	}
	if (!value_extract(state, result, sizeof(*result))) {
		ZCBOR_FAIL();
	}
	return true;
}


bool double_expect(zcbor_state_t *state, double *result)
{
	double value;

	if (!double_decode(state, &value)) {
		ZCBOR_FAIL();
	}
	if (value != *result) {
		FAIL_RESTORE();
	}
	return true;
}


bool zcbor_any_decode(zcbor_state_t *state, void *result)
{
	zcbor_assert(result == NULL,
			"'any' type cannot be returned, only skipped.\n");

	FAIL_IF(state->payload >= state->payload_end);
	uint8_t major_type = MAJOR_TYPE(*state->payload);
	uint32_t value;
	uint32_t num_decode;
	uint32_t temp_elem_count;
	uint8_t const *payload_bak;

	if (!value_extract(state, &value, sizeof(value))) {
		/* Can happen because of elem_count (or payload_end) */
		/* Note: Indeterminate length arrays are not supported in zcbor_any_decode */
		ZCBOR_FAIL();
	}

	switch (major_type) {
		case ZCBOR_MAJOR_TYPE_BSTR:
		case ZCBOR_MAJOR_TYPE_TSTR:
			/* 'value' is the length of the BSTR or TSTR */
			if (value > (state->payload_end - state->payload)) {
				ZCBOR_FAIL();
			}
			(state->payload) += value;
			break;
		case ZCBOR_MAJOR_TYPE_MAP:
			value *= 2; /* Because all members have a key. */
			/* Fallthrough */
		case ZCBOR_MAJOR_TYPE_LIST:
			temp_elem_count = state->elem_count;
			payload_bak = state->payload;
			state->elem_count = value;
			if (!zcbor_multi_decode(value, value, &num_decode,
					(void *)zcbor_any_decode, state,
					NULL, 0)) {
				state->elem_count = temp_elem_count;
				state->payload = payload_bak;
				ZCBOR_FAIL();
			}
			state->elem_count = temp_elem_count;
			break;
		default:
			/* Do nothing */
			break;
	}

	return true;
}


bool zcbor_tag_decode(zcbor_state_t *state, uint32_t *result)
{
	FAIL_IF(state->payload >= state->payload_end);
	uint8_t major_type = MAJOR_TYPE(*state->payload);

	if (major_type != ZCBOR_MAJOR_TYPE_TAG) {
		/* Value to be read doesn't have the right type. */
		ZCBOR_FAIL();
	}
	if (!value_extract(state, result, sizeof(*result))) {
		ZCBOR_FAIL();
	}
	state->elem_count++;
	return true;
}


bool zcbor_tag_expect(zcbor_state_t *state, uint32_t result)
{
	uint32_t tag_val;

	if (!zcbor_tag_decode(state, &tag_val)) {
		ZCBOR_FAIL();
	}
	if (tag_val != result) {
		FAIL_RESTORE();
	}
	return true;
}


bool zcbor_multi_decode(uint32_t min_decode,
		uint32_t max_decode,
		uint32_t *num_decode,
		zcbor_decoder_t decoder,
		zcbor_state_t *state,
		void *result,
		uint32_t result_len)
{
	for (uint32_t i = 0; i < max_decode; i++) {
		uint8_t const *payload_bak = state->payload;
		uint32_t elem_count_bak = state->elem_count;

		if (!decoder(state,
				(uint8_t *)result + i*result_len)) {
			*num_decode = i;
			state->payload = payload_bak;
			state->elem_count = elem_count_bak;
			if (i < min_decode) {
				ZCBOR_FAIL();
			} else {
				zcbor_print("Found %d elements.\n", i);
			}
			return true;
		}
	}
	zcbor_print("Found %d elements.\n", max_decode);
	*num_decode = max_decode;
	return true;
}


bool zcbor_present_decode(uint32_t *present,
		zcbor_decoder_t decoder,
		zcbor_state_t *state,
		void *result)
{
	uint32_t num_decode;
	bool retval = zcbor_multi_decode(0, 1, &num_decode, decoder, state, result, 0);
	if (retval) {
		*present = num_decode;
	}
	return retval;
}
