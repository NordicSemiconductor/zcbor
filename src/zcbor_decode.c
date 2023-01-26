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
static size_t additional_len(uint8_t additional)
{
	if (ZCBOR_VALUE_IS_1_BYTE <= additional && additional <= ZCBOR_VALUE_IS_8_BYTES) {
		/* 24 => 1
		 * 25 => 2
		 * 26 => 4
		 * 27 => 8
		 */
		return 1U << (additional - ZCBOR_VALUE_IS_1_BYTE);
	}
	return 0;
}

/** Extract the major type, i.e. the first 3 bits of the header byte. */
#define MAJOR_TYPE(header_byte) ((zcbor_major_type_t)(((header_byte) >> 5) & 0x7))

/** Extract the additional info, i.e. the last 5 bits of the header byte. */
#define ADDITIONAL(header_byte) ((header_byte) & 0x1F)


#define FAIL_AND_DECR_IF(expr, err) \
do {\
	if (expr) { \
		(state->payload)--; \
		ZCBOR_ERR(err); \
	} \
} while(0)


static bool initial_checks(zcbor_state_t *state)
{
	ZCBOR_CHECK_ERROR();
	ZCBOR_CHECK_PAYLOAD();
	return true;
}


static bool type_check(zcbor_state_t *state, zcbor_major_type_t exp_major_type)
{
	if (!initial_checks(state)) {
		ZCBOR_FAIL();
	}
	zcbor_major_type_t major_type = MAJOR_TYPE(*state->payload);

	if (major_type != exp_major_type) {
		ZCBOR_ERR(ZCBOR_ERR_WRONG_TYPE);
	}
	return true;
}


#define INITIAL_CHECKS() \
do {\
	if (!initial_checks(state)) { \
		ZCBOR_FAIL(); \
	} \
} while(0)

#define INITIAL_CHECKS_WITH_TYPE(exp_major_type) \
do {\
	if (!type_check(state, exp_major_type)) { \
		ZCBOR_FAIL(); \
	} \
} while(0)

static void err_restore(zcbor_state_t *state, int err)
{
	state->payload = state->payload_bak;
	state->elem_count++;
	zcbor_error(state, err);
}

#define ERR_RESTORE(err) \
do { \
	err_restore(state, err); \
	ZCBOR_FAIL(); \
} while(0)

#define FAIL_RESTORE() \
do { \
	state->payload = state->payload_bak; \
	state->elem_count++; \
	ZCBOR_FAIL(); \
} while(0)


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
		void *const result, size_t result_len)
{
	zcbor_trace();
	zcbor_assert_state(result_len != 0, "0-length result not supported.\r\n");
	zcbor_assert_state(result != NULL, NULL);

	INITIAL_CHECKS();
	ZCBOR_ERR_IF((state->elem_count == 0), ZCBOR_ERR_LOW_ELEM_COUNT);

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
		size_t len = additional_len(additional);

		FAIL_AND_DECR_IF(len > result_len, ZCBOR_ERR_INT_SIZE);
		FAIL_AND_DECR_IF(len == 0, ZCBOR_ERR_ADDITIONAL_INVAL); // additional_len() did not recognize the additional value.
		FAIL_AND_DECR_IF((state->payload + len) > state->payload_end,
			ZCBOR_ERR_NO_PAYLOAD);

#ifdef CONFIG_BIG_ENDIAN
		memcpy(&u8_result[result_len - len], state->payload, len);
#else
		for (size_t i = 0; i < len; i++) {
			u8_result[i] = (state->payload)[len - i - 1];
		}
#endif /* CONFIG_BIG_ENDIAN */

		(state->payload) += len;
	}

	(state->elem_count)--;
	return true;
}


bool zcbor_int_decode(zcbor_state_t *state, void *result_int, size_t int_size)
{
	INITIAL_CHECKS();
	zcbor_major_type_t major_type = MAJOR_TYPE(*state->payload);
	uint8_t *result_uint8 = (uint8_t *)result_int;
	int8_t *result_int8 = (int8_t *)result_int;

	if (major_type != ZCBOR_MAJOR_TYPE_PINT
		&& major_type != ZCBOR_MAJOR_TYPE_NINT) {
		/* Value to be read doesn't have the right type. */
		ZCBOR_ERR(ZCBOR_ERR_WRONG_TYPE);
	}

	if (!value_extract(state, result_int, int_size)) {
		ZCBOR_FAIL();
	}

#ifdef CONFIG_BIG_ENDIAN
	if (result_int8[0] < 0) {
#else
	if (result_int8[int_size - 1] < 0) {
#endif
		/* Value is too large to fit in a signed integer. */
		ERR_RESTORE(ZCBOR_ERR_INT_SIZE);
	}

	if (major_type == ZCBOR_MAJOR_TYPE_NINT) {
		/* Convert from CBOR's representation by flipping all bits. */
		for (unsigned int i = 0; i < int_size; i++) {
			result_uint8[i] = (uint8_t)~result_uint8[i];
		}
	}

	return true;
}


bool zcbor_int32_decode(zcbor_state_t *state, int32_t *result)
{
	return zcbor_int_decode(state, result, sizeof(*result));
}


bool zcbor_int64_decode(zcbor_state_t *state, int64_t *result)
{
	return zcbor_int_decode(state, result, sizeof(*result));
}


bool zcbor_uint_decode(zcbor_state_t *state, void *result_uint, size_t uint_size)
{
	INITIAL_CHECKS_WITH_TYPE(ZCBOR_MAJOR_TYPE_PINT);

	if (!value_extract(state, result_uint, uint_size)) {
		zcbor_print("uint with size %d failed.\r\n", uint_size);
		ZCBOR_FAIL();
	}
	return true;
}


bool zcbor_uint32_decode(zcbor_state_t *state, uint32_t *result)
{
	return zcbor_uint_decode(state, result, sizeof(*result));
}


bool zcbor_int32_expect_union(zcbor_state_t *state, int32_t result)
{
	if (!zcbor_union_elem_code(state)) {
		ZCBOR_FAIL();
	}
	return zcbor_int32_expect(state, result);
}


bool zcbor_int64_expect_union(zcbor_state_t *state, int64_t result)
{
	if (!zcbor_union_elem_code(state)) {
		ZCBOR_FAIL();
	}
	return zcbor_int64_expect(state, result);
}


bool zcbor_uint32_expect_union(zcbor_state_t *state, uint32_t result)
{
	if (!zcbor_union_elem_code(state)) {
		ZCBOR_FAIL();
	}
	return zcbor_uint32_expect(state, result);
}


bool zcbor_uint64_expect_union(zcbor_state_t *state, uint64_t result)
{
	if (!zcbor_union_elem_code(state)) {
		ZCBOR_FAIL();
	}
	return zcbor_uint64_expect(state, result);
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
		ERR_RESTORE(ZCBOR_ERR_WRONG_VALUE);
	}
	return true;
}


bool zcbor_uint64_decode(zcbor_state_t *state, uint64_t *result)
{
	return zcbor_uint_decode(state, result, sizeof(*result));
}


#ifdef ZCBOR_SUPPORTS_SIZE_T
bool zcbor_size_decode(zcbor_state_t *state, size_t *result)
{
	return zcbor_uint_decode(state, result, sizeof(*result));
}
#endif


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
		ERR_RESTORE(ZCBOR_ERR_WRONG_VALUE);
	}
	return true;
}


#ifdef ZCBOR_SUPPORTS_SIZE_T
bool zcbor_size_expect(zcbor_state_t *state, size_t result)
{
	return zcbor_uint64_expect(state, result);
}
#endif


static bool str_start_decode(zcbor_state_t *state,
		struct zcbor_string *result, zcbor_major_type_t exp_major_type)
{
	INITIAL_CHECKS_WITH_TYPE(exp_major_type);

	if (!value_extract(state, &result->len, sizeof(result->len))) {
		ZCBOR_FAIL();
	}

	result->value = state->payload;
	return true;
}

static bool str_start_decode_with_overflow_check(zcbor_state_t *state,
		struct zcbor_string *result, zcbor_major_type_t exp_major_type)
{
	bool res = str_start_decode(state, result, exp_major_type);

	if (!res) {
		ZCBOR_FAIL();
	}

	/* Casting to size_t is safe since str_start_decode() checks that
	 * payload_end is bigger that payload. */
	if (result->len > (size_t)(state->payload_end - state->payload)) {
		zcbor_print("error: 0x%zu > 0x%zu\r\n",
			result->len,
			(state->payload_end - state->payload));
		ERR_RESTORE(ZCBOR_ERR_NO_PAYLOAD);
	}

	return true;
}


bool zcbor_bstr_start_decode(zcbor_state_t *state, struct zcbor_string *result)
{
	struct zcbor_string dummy;
	if (result == NULL) {
		result = &dummy;
	}

	if(!str_start_decode_with_overflow_check(state, result, ZCBOR_MAJOR_TYPE_BSTR)) {
		ZCBOR_FAIL();
	}

	if (!zcbor_new_backup(state, ZCBOR_MAX_ELEM_COUNT)) {
		FAIL_RESTORE();
	}

	state->payload_end = result->value + result->len;
	return true;
}


bool zcbor_bstr_end_decode(zcbor_state_t *state)
{
	ZCBOR_ERR_IF(state->payload != state->payload_end, ZCBOR_ERR_PAYLOAD_NOT_CONSUMED);

	if (!zcbor_process_backup(state,
			ZCBOR_FLAG_RESTORE | ZCBOR_FLAG_CONSUME | ZCBOR_FLAG_TRANSFER_PAYLOAD,
			ZCBOR_MAX_ELEM_COUNT)) {
		ZCBOR_FAIL();
	}

	return true;
}


static void partition_fragment(const zcbor_state_t *state,
	struct zcbor_string_fragment *result)
{
	result->fragment.len = MIN(result->fragment.len,
		(size_t)state->payload_end - (size_t)state->payload);
}


static bool start_decode_fragment(zcbor_state_t *state,
	struct zcbor_string_fragment *result,
	zcbor_major_type_t exp_major_type)
{
	if(!str_start_decode(state, &result->fragment, exp_major_type)) {
		ZCBOR_FAIL();
	}

	result->offset = 0;
	result->total_len = result->fragment.len;
	partition_fragment(state, result);
	state->payload_end = state->payload + result->fragment.len;

	return true;
}

bool zcbor_bstr_start_decode_fragment(zcbor_state_t *state,
	struct zcbor_string_fragment *result)
{
	if (!start_decode_fragment(state, result, ZCBOR_MAJOR_TYPE_BSTR)) {
		ZCBOR_FAIL();
	}
	if (!zcbor_new_backup(state, ZCBOR_MAX_ELEM_COUNT)) {
		FAIL_RESTORE();
	}
	return true;
}


void zcbor_next_fragment(zcbor_state_t *state,
	struct zcbor_string_fragment *prev_fragment,
	struct zcbor_string_fragment *result)
{
	memcpy(result, prev_fragment, sizeof(*result));
	result->fragment.value = state->payload_mut;
	result->offset += prev_fragment->fragment.len;
	result->fragment.len = result->total_len - result->offset;

	partition_fragment(state, result);
	zcbor_print("New fragment length %zu\r\n", result->fragment.len);

	state->payload += result->fragment.len;
}


void zcbor_bstr_next_fragment(zcbor_state_t *state,
	struct zcbor_string_fragment *prev_fragment,
	struct zcbor_string_fragment *result)
{
	memcpy(result, prev_fragment, sizeof(*result));
	result->fragment.value = state->payload_mut;
	result->offset += prev_fragment->fragment.len;
	result->fragment.len = result->total_len - result->offset;

	partition_fragment(state, result);
	zcbor_print("fragment length %zu\r\n", result->fragment.len);
	state->payload_end = state->payload + result->fragment.len;
}


bool zcbor_is_last_fragment(const struct zcbor_string_fragment *fragment)
{
	return (fragment->total_len == (fragment->offset + fragment->fragment.len));
}


static bool str_decode(zcbor_state_t *state, struct zcbor_string *result,
		zcbor_major_type_t exp_major_type)
{
	if (!str_start_decode_with_overflow_check(state, result, exp_major_type)) {
		ZCBOR_FAIL();
	}

	state->payload += result->len;
	return true;
}


static bool str_decode_fragment(zcbor_state_t *state, struct zcbor_string_fragment *result,
		zcbor_major_type_t exp_major_type)
{
	if (!start_decode_fragment(state, result, exp_major_type)) {
		ZCBOR_FAIL();
	}

	(state->payload) += result->fragment.len;
	return true;
}


static bool str_expect(zcbor_state_t *state, struct zcbor_string *result,
		zcbor_major_type_t exp_major_type)
{
	struct zcbor_string tmp_result;

	if (!str_decode(state, &tmp_result, exp_major_type)) {
		ZCBOR_FAIL();
	}
	if (!zcbor_compare_strings(&tmp_result, result)) {
		ERR_RESTORE(ZCBOR_ERR_WRONG_VALUE);
	}
	return true;
}


bool zcbor_bstr_decode(zcbor_state_t *state, struct zcbor_string *result)
{
	return str_decode(state, result, ZCBOR_MAJOR_TYPE_BSTR);
}


bool zcbor_bstr_decode_fragment(zcbor_state_t *state, struct zcbor_string_fragment *result)
{
	return str_decode_fragment(state, result, ZCBOR_MAJOR_TYPE_BSTR);
}


bool zcbor_bstr_expect(zcbor_state_t *state, struct zcbor_string *result)
{
	return str_expect(state, result, ZCBOR_MAJOR_TYPE_BSTR);
}


bool zcbor_tstr_decode(zcbor_state_t *state, struct zcbor_string *result)
{
	return str_decode(state, result, ZCBOR_MAJOR_TYPE_TSTR);
}


bool zcbor_tstr_decode_fragment(zcbor_state_t *state, struct zcbor_string_fragment *result)
{
	return str_decode_fragment(state, result, ZCBOR_MAJOR_TYPE_TSTR);
}


bool zcbor_tstr_expect(zcbor_state_t *state, struct zcbor_string *result)
{
	return str_expect(state, result, ZCBOR_MAJOR_TYPE_TSTR);
}


static bool list_map_start_decode(zcbor_state_t *state,
		zcbor_major_type_t exp_major_type)
{
	size_t new_elem_count;
	bool indefinite_length_array = false;

	INITIAL_CHECKS_WITH_TYPE(exp_major_type);

	if (ADDITIONAL(*state->payload) == ZCBOR_VALUE_IS_INDEFINITE_LENGTH) {
		/* Indefinite length array. */
		new_elem_count = ZCBOR_LARGE_ELEM_COUNT;
		ZCBOR_ERR_IF(state->elem_count == 0, ZCBOR_ERR_LOW_ELEM_COUNT);
		indefinite_length_array = true;
		state->payload++;
		state->elem_count--;
	} else {
		if (!value_extract(state, &new_elem_count, sizeof(new_elem_count))) {
			ZCBOR_FAIL();
		}
	}

	if (!zcbor_new_backup(state, new_elem_count)) {
		FAIL_RESTORE();
	}

	state->indefinite_length_array = indefinite_length_array;

	return true;
}


bool zcbor_list_start_decode(zcbor_state_t *state)
{
	return list_map_start_decode(state, ZCBOR_MAJOR_TYPE_LIST);
}


bool zcbor_map_start_decode(zcbor_state_t *state)
{
	bool ret = list_map_start_decode(state, ZCBOR_MAJOR_TYPE_MAP);

	if (ret && !state->indefinite_length_array) {
		if (state->elem_count >= (ZCBOR_MAX_ELEM_COUNT / 2)) {
			/* The new elem_count is too large. */
			ERR_RESTORE(ZCBOR_ERR_INT_SIZE);
		}
		state->elem_count *= 2;
	}
	return ret;
}


static bool array_end_expect(zcbor_state_t *state)
{
	INITIAL_CHECKS();
	ZCBOR_ERR_IF(*state->payload != 0xFF, ZCBOR_ERR_WRONG_TYPE);

	state->payload++;
	return true;
}


static bool list_map_end_decode(zcbor_state_t *state)
{
	size_t max_elem_count = 0;

	if (state->indefinite_length_array) {
		if (!array_end_expect(state)) {
			ZCBOR_FAIL();
		}
		max_elem_count = ZCBOR_MAX_ELEM_COUNT;
		state->indefinite_length_array = false;
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
	return list_map_end_decode(state);
}


bool zcbor_map_end_decode(zcbor_state_t *state)
{
	return list_map_end_decode(state);
}


bool zcbor_list_map_end_force_decode(zcbor_state_t *state)
{
	if (!zcbor_process_backup(state,
			ZCBOR_FLAG_RESTORE | ZCBOR_FLAG_CONSUME | ZCBOR_FLAG_TRANSFER_PAYLOAD,
			ZCBOR_MAX_ELEM_COUNT)) {
		ZCBOR_FAIL();
	}

	return true;
}


bool zcbor_simple_decode(zcbor_state_t *state, uint8_t *result)
{
	INITIAL_CHECKS_WITH_TYPE(ZCBOR_MAJOR_TYPE_SIMPLE);

	/* Simple values must be 0-23 (additional is 0-23) or 24-255 (additional is 24).
	 * Other additional values are not considered simple values. */
	ZCBOR_ERR_IF(ADDITIONAL(*state->payload) > 24, ZCBOR_ERR_WRONG_TYPE);

	if (!value_extract(state, result, sizeof(*result))) {
		ZCBOR_FAIL();
	}
	return true;
}


bool zcbor_simple_expect(zcbor_state_t *state, uint8_t result)
{
	uint8_t value;

	if (!zcbor_simple_decode(state, &value)) {
		ZCBOR_FAIL();
	}

	if (value != result) {
		zcbor_print("simple value %u != %u\r\n", value, result);
		ERR_RESTORE(ZCBOR_ERR_WRONG_VALUE);
	}

	return true;
}


bool zcbor_nil_expect(zcbor_state_t *state, void *unused)
{
	(void)unused;
	return zcbor_simple_expect(state, 22);
}


bool zcbor_undefined_expect(zcbor_state_t *state, void *unused)
{
	(void)unused;
	return zcbor_simple_expect(state, 23);
}


bool zcbor_bool_decode(zcbor_state_t *state, bool *result)
{
	uint8_t value;

	if (!zcbor_simple_decode(state, &value)) {
		ZCBOR_FAIL();
	}
	value -= ZCBOR_BOOL_TO_SIMPLE;
	if (value > 1) {
		ERR_RESTORE(ZCBOR_ERR_WRONG_TYPE);
	}
	*result = value;

	zcbor_print("boolval: %u\r\n", *result);
	return true;
}


bool zcbor_bool_expect(zcbor_state_t *state, bool result)
{
	return zcbor_simple_expect(state, (uint8_t)(!!result) + ZCBOR_BOOL_TO_SIMPLE);
}


static bool float_check(zcbor_state_t *state, uint8_t additional_val)
{
	INITIAL_CHECKS_WITH_TYPE(ZCBOR_MAJOR_TYPE_SIMPLE);
	ZCBOR_ERR_IF(ADDITIONAL(*state->payload) != additional_val, ZCBOR_ERR_FLOAT_SIZE);
	return true;
}


bool zcbor_float16_bytes_decode(zcbor_state_t *state, uint16_t *result)
{

	ZCBOR_FAIL_IF(!float_check(state, ZCBOR_VALUE_IS_2_BYTES));

	if (!value_extract(state, result, sizeof(*result))) {
		ZCBOR_FAIL();
	}

	return true;
}


bool zcbor_float16_bytes_expect(zcbor_state_t *state, uint16_t result)
{
	uint16_t value;

	if (!zcbor_float16_bytes_decode(state, &value)) {
		ZCBOR_FAIL();
	}
	if (value != result) {
		ERR_RESTORE(ZCBOR_ERR_WRONG_VALUE);
	}
	return true;
}


/* Float16: */
#define F16_SIGN_OFFS 15 /* Bit offset of the sign bit. */
#define F16_EXPO_OFFS 10 /* Bit offset of the exponent. */
#define F16_EXPO_MSK 0x1F /* Bitmask for the exponent (right shifted by F16_EXPO_OFFS). */
#define F16_MANTISSA_MSK 0x3FF /* Bitmask for the mantissa. */
#define F16_MIN_EXPO 24 /* Negative exponent of the non-zero float16 value closest to 0 (2^-24) */
#define F16_MIN (1.0 / (1 << F16_MIN_EXPO)) /* The non-zero float16 value closest to 0 (2^-24) */
#define F16_BIAS 15 /* The exponent bias of normalized float16 values. */

/* Float32: */
#define F32_SIGN_OFFS 31 /* Bit offset of the sign bit. */
#define F32_EXPO_OFFS 23 /* Bit offset of the exponent. */
#define F32_EXPO_MSK 0xFF /* Bitmask for the exponent (right shifted by F32_EXPO_OFFS). */
#define F32_BIAS 127 /* The exponent bias of normalized float32 values. */


bool zcbor_float16_decode(zcbor_state_t *state, float *result)
{
	uint16_t value16;

	if (!zcbor_float16_bytes_decode(state, &value16)) {
		ZCBOR_FAIL();
	}

	uint32_t sign = value16 >> F16_SIGN_OFFS;
	uint32_t expo = (value16 >> F16_EXPO_OFFS) & F16_EXPO_MSK;
	uint32_t mantissa = value16 & F16_MANTISSA_MSK;

	if ((expo == 0) && (mantissa != 0)) {
		/* Subnormal float16 - convert to normalized float32 */
		*result = ((float)mantissa * (float)F16_MIN) * (sign ? -1 : 1);
	} else {
		/* Normalized / zero / Infinity / NaN */
		uint32_t new_expo = (expo == 0 /* zero */) ? 0
			: (expo == F16_EXPO_MSK /* inf/NaN */) ? F32_EXPO_MSK
				: (expo + (F32_BIAS - F16_BIAS));
		uint32_t value32 = (sign << F32_SIGN_OFFS) | (new_expo << F32_EXPO_OFFS)
			| (mantissa << (F32_EXPO_OFFS - F16_EXPO_OFFS));
		memcpy(result, &value32, sizeof(*result));
	}

	return true;
}


bool zcbor_float16_expect(zcbor_state_t *state, float result)
{
	float value;

	if (!zcbor_float16_decode(state, &value)) {
		ZCBOR_FAIL();
	}
	if (value != result) {
		ERR_RESTORE(ZCBOR_ERR_WRONG_VALUE);
	}
	return true;
}


bool zcbor_float32_decode(zcbor_state_t *state, float *result)
{
	ZCBOR_FAIL_IF(!float_check(state, ZCBOR_VALUE_IS_4_BYTES));

	if (!value_extract(state, result, sizeof(*result))) {
		ZCBOR_FAIL();
	}

	return true;
}


bool zcbor_float32_expect(zcbor_state_t *state, float result)
{
	float value;

	if (!zcbor_float32_decode(state, &value)) {
		ZCBOR_FAIL();
	}
	if (value != result) {
		ERR_RESTORE(ZCBOR_ERR_WRONG_VALUE);
	}
	return true;
}


bool zcbor_float16_32_decode(zcbor_state_t *state, float *result)
{
	if (zcbor_float16_decode(state, result)) {
		/* Do nothing */
	} else if (!zcbor_float32_decode(state, result)) {
		ZCBOR_FAIL();
	}

	return true;
}


bool zcbor_float16_32_expect(zcbor_state_t *state, float result)
{
	if (zcbor_float16_expect(state, (float)result)) {
		/* Do nothing */
	} else if (!zcbor_float32_expect(state, result)) {
		ZCBOR_FAIL();
	}

	return true;
}


bool zcbor_float64_decode(zcbor_state_t *state, double *result)
{
	ZCBOR_FAIL_IF(!float_check(state, ZCBOR_VALUE_IS_8_BYTES));

	if (!value_extract(state, result, sizeof(*result))) {
		ZCBOR_FAIL();
	}

	return true;
}


bool zcbor_float64_expect(zcbor_state_t *state, double result)
{
	double value;

	if (!zcbor_float64_decode(state, &value)) {
		ZCBOR_FAIL();
	}
	if (value != result) {
		ERR_RESTORE(ZCBOR_ERR_WRONG_VALUE);
	}
	return true;
}


bool zcbor_float32_64_decode(zcbor_state_t *state, double *result)
{
	float float_result;

	if (zcbor_float32_decode(state, &float_result)) {
		*result = (double)float_result;
	} else if (!zcbor_float64_decode(state, result)) {
		ZCBOR_FAIL();
	}

	return true;
}


bool zcbor_float32_64_expect(zcbor_state_t *state, double result)
{
	if (zcbor_float64_expect(state, result)) {
		/* Do nothing */
	} else if (!zcbor_float32_expect(state, (float)result)) {
		ZCBOR_FAIL();
	}

	return true;
}


bool zcbor_float_decode(zcbor_state_t *state, double *result)
{
	float float_result;

	if (zcbor_float16_decode(state, &float_result)) {
		*result = (double)float_result;
	} else if (zcbor_float32_decode(state, &float_result)) {
		*result = (double)float_result;
	} else if (!zcbor_float64_decode(state, result)) {
		ZCBOR_FAIL();
	}

	return true;
}


bool zcbor_float_expect(zcbor_state_t *state, double result)
{
	if (zcbor_float16_expect(state, (float)result)) {
		/* Do nothing */
	} else if (zcbor_float32_expect(state, (float)result)) {
		/* Do nothing */
	} else if (!zcbor_float64_expect(state, result)) {
		ZCBOR_FAIL();
	}

	return true;
}


bool zcbor_any_skip(zcbor_state_t *state, void *result)
{
	zcbor_assert_state(result == NULL,
			"'any' type cannot be returned, only skipped.\r\n");
	(void)result;

	INITIAL_CHECKS();
	zcbor_major_type_t major_type = MAJOR_TYPE(*state->payload);
	uint8_t additional = ADDITIONAL(*state->payload);
	uint64_t value = 0; /* In case of indefinite_length_array. */
	zcbor_state_t state_copy;

	memcpy(&state_copy, state, sizeof(zcbor_state_t));

	while (major_type == ZCBOR_MAJOR_TYPE_TAG) {
		uint32_t tag_dummy;

		if (!zcbor_tag_decode(&state_copy, &tag_dummy)) {
			ZCBOR_FAIL();
		}
		ZCBOR_ERR_IF(state_copy.payload >= state_copy.payload_end, ZCBOR_ERR_NO_PAYLOAD);
		major_type = MAJOR_TYPE(*state_copy.payload);
		additional = ADDITIONAL(*state_copy.payload);
	}

	const bool indefinite_length_array = ((additional == ZCBOR_VALUE_IS_INDEFINITE_LENGTH)
		&& ((major_type == ZCBOR_MAJOR_TYPE_LIST) || (major_type == ZCBOR_MAJOR_TYPE_MAP)));

	if (!indefinite_length_array && !value_extract(&state_copy, &value, sizeof(value))) {
		/* Can happen because of elem_count (or payload_end) */
		ZCBOR_FAIL();
	}

	switch (major_type) {
		case ZCBOR_MAJOR_TYPE_BSTR:
		case ZCBOR_MAJOR_TYPE_TSTR:
			/* 'value' is the length of the BSTR or TSTR.
			 * The cast to size_t is safe because value_extract() above
			 * checks that payload_end is greater than payload. */
			ZCBOR_ERR_IF(
				value > (uint64_t)(state_copy.payload_end - state_copy.payload),
				ZCBOR_ERR_NO_PAYLOAD);
			(state_copy.payload) += value;
			break;
		case ZCBOR_MAJOR_TYPE_MAP:
			ZCBOR_ERR_IF(value > (SIZE_MAX / 2), ZCBOR_ERR_INT_SIZE);
			value *= 2;
			/* fallthrough */
		case ZCBOR_MAJOR_TYPE_LIST:
			if (indefinite_length_array) {
				state_copy.payload++;
				value = ZCBOR_LARGE_ELEM_COUNT;
			}
			state_copy.elem_count = (size_t)value;
			state_copy.indefinite_length_array = indefinite_length_array;
			while (!zcbor_array_at_end(&state_copy)) {
				if (!zcbor_any_skip(&state_copy, NULL)) {
					ZCBOR_FAIL();
				}
			}
			if (indefinite_length_array && !array_end_expect(&state_copy)) {
				ZCBOR_FAIL();
			}
			break;
		default:
			/* Do nothing */
			break;
	}

	state->payload = state_copy.payload;
	state->elem_count--;

	return true;
}


bool zcbor_tag_decode(zcbor_state_t *state, uint32_t *result)
{
	INITIAL_CHECKS_WITH_TYPE(ZCBOR_MAJOR_TYPE_TAG);

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
		ERR_RESTORE(ZCBOR_ERR_WRONG_VALUE);
	}
	return true;
}


bool zcbor_multi_decode(size_t min_decode,
		size_t max_decode,
		size_t *num_decode,
		zcbor_decoder_t decoder,
		zcbor_state_t *state,
		void *result,
		size_t result_len)
{
	ZCBOR_CHECK_ERROR();
	for (size_t i = 0; i < max_decode; i++) {
		uint8_t const *payload_bak = state->payload;
		size_t elem_count_bak = state->elem_count;

		if (!decoder(state,
				(uint8_t *)result + i*result_len)) {
			*num_decode = i;
			state->payload = payload_bak;
			state->elem_count = elem_count_bak;
			ZCBOR_ERR_IF(i < min_decode, ZCBOR_ERR_ITERATIONS);
			zcbor_print("Found %" PRIuFAST32 " elements.\r\n", i);
			return true;
		}
	}
	zcbor_print("Found %" PRIuFAST32 " elements.\r\n", max_decode);
	*num_decode = max_decode;
	return true;
}


bool zcbor_present_decode(bool *present,
		zcbor_decoder_t decoder,
		zcbor_state_t *state,
		void *result)
{
	size_t num_decode;
	bool retval = zcbor_multi_decode(0, 1, &num_decode, decoder, state, result, 0);

	zcbor_assert_state(retval, "zcbor_multi_decode should not fail with these parameters.\r\n");

	*present = !!num_decode;
	return retval;
}


void zcbor_new_decode_state(zcbor_state_t *state_array, size_t n_states,
		const uint8_t *payload, size_t payload_len, size_t elem_count)
{
	zcbor_new_state(state_array, n_states, payload, payload_len, elem_count);
}
