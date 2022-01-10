/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CBOR_ENCODE_H__
#define CBOR_ENCODE_H__
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "zcbor_common.h"


/** Encode a PINT/NINT.
 *
 * @param[inout] state        The current state of the decoding.
 * @param[out]   result       Where to place the encoded value.
 *
 * @retval true   Everything is ok.
 * @retval false  If the payload is exhausted.
 */
bool zcbor_int32_encode(zcbor_state_t *state, const int32_t *input);
bool zcbor_int64_encode(zcbor_state_t *state, const int64_t *input);
bool zcbor_int32_put(zcbor_state_t *state, int32_t result);
bool zcbor_int64_put(zcbor_state_t *state, int64_t input);

/** Encode a PINT. */
bool zcbor_uint32_encode(zcbor_state_t *state, const uint32_t *result);
bool zcbor_uint64_encode(zcbor_state_t *state, const uint64_t *input);
bool zcbor_uint32_put(zcbor_state_t *state, uint32_t result);
bool zcbor_uint64_put(zcbor_state_t *state, uint64_t input);

/** Encode a BSTR header.
 *
 * The rest of the string can be encoded as CBOR.
 * A state backup is created to keep track of the element count.
 *
 * @retval true   Header encoded correctly
 * @retval false  Header encoded incorrectly, or backup failed.
 */
bool zcbor_bstr_start_encode(zcbor_state_t *state, const zcbor_string_type_t *result);

/** Finalize encoding a CBOR-encoded BSTR.
 *
 * Restore element count from backup.
 */
bool zcbor_bstr_end_encode(zcbor_state_t *state);

/** Encode a BSTR, */
bool zcbor_bstr_encode(zcbor_state_t *state, const zcbor_string_type_t *result);

/** Encode a TSTR. */
bool zcbor_tstr_encode(zcbor_state_t *state, const zcbor_string_type_t *result);

#define zcbor_tstr_put(state, string) \
	zcbor_tstr_encode(state, &(zcbor_string_type_t){.value = string, .len = (sizeof(string) - 1)})

#define zcbor_tstr_put_term(state, string) \
	zcbor_tstr_encode(state, &(zcbor_string_type_t){.value = string, .len = strlen(string)})

/** Encode a LIST header.
 *
 * The contents of the list can be decoded via subsequent function calls.
 * A state backup is created to keep track of the element count.
 */
bool zcbor_list_start_encode(zcbor_state_t *state, uint32_t max_num);

/** Encode a MAP header. */
bool zcbor_map_start_encode(zcbor_state_t *state, uint32_t max_num);

/** Encode end of a LIST. Do some checks and deallocate backup. */
bool zcbor_list_end_encode(zcbor_state_t *state, uint32_t max_num);

/** Encode end of a MAP. Do some checks and deallocate backup. */
bool zcbor_map_end_encode(zcbor_state_t *state, uint32_t max_num);

/** Encode a "nil" primitive value. result should be NULL. */
bool zcbor_nil_put(zcbor_state_t *state, const void *result);

/** Encode a boolean primitive value. */
bool zcbor_bool_encode(zcbor_state_t *state, const bool *result);
bool zcbor_bool_put(zcbor_state_t *state, bool result);

/** Encode a float */
bool zcbor_float_encode(zcbor_state_t *state, double *result);
bool zcbor_float_put(zcbor_state_t *state, double result);

/** Dummy encode "any": Encode a "nil". input should be NULL. */
bool zcbor_any_encode(zcbor_state_t *state, void *input);

/** Encode a tag. */
bool zcbor_tag_encode(zcbor_state_t *state, uint32_t tag);

/** Encode 0 or more elements with the same type and constraints.
 *
 * @details This must not necessarily encode all elements in a list. E.g. if
 *          the list contains 3 INTS between 0 and 100 followed by 0 to 2 BSTRs
 *          with length 8, that could be done with:
 *
 *          @code{c}
 *              uint32_t int_min = 0;
 *              uint32_t int_max = 100;
 *              uint32_t bstr_size = 8;
 *              uint32_t ints[3];
 *              zcbor_string_type_t bstrs[2] = <initialize here>;
 *              bool res;
 *
 *              res = zcbor_list_start_encode(state, 5);
 *              // check res
 *              res = zcbor_multi_encode(3, 3, &num_encode, zcbor_uint32_encode, state,
 *                           ints, 4);
 *              // check res
 *              res = zcbor_multi_encode(0, 2, &num_encode, strx_encode, state,
 *                           bstrs, sizeof(zcbor_string_type_t));
 *              // check res
 *              res = zcbor_list_end_encode(state, 5);
 *              // check res
 *          @endcode
 *
 * @param[in]  min_encode    The minimum acceptable number of elements.
 * @param[in]  max_encode    The maximum acceptable number of elements.
 * @param[in]  num_encode    The actual number of elements.
 * @param[in]  encoder       The encoder function to call under the hood. This
 *                           function will be called with the provided arguments
 *                           repeatedly until the function fails (returns false)
 *                           or until it has been called @p max_encode times.
 *                           result is moved @p result_len bytes for each call
 *                           to @p encoder, i.e. @p result refers to an array
 *                           of result variables.
 * @param[in]  input         Source of the encoded values. Must be an array
 *                           of length at least @p max_encode.
 * @param[in]  result_len    The length of the result variables. Must be the
 *                           length of the elements in result.
 *
 * @retval true   If at least @p min_encode variables were correctly encoded.
 * @retval false  If @p encoder failed before having encoded @p min_encode
 *                values.
 */
bool zcbor_multi_encode(uint32_t min_encode, uint32_t max_encode, const uint32_t *num_encode,
		zcbor_encoder_t encoder, zcbor_state_t *state, const void *input,
		uint32_t result_len);

bool zcbor_present_encode(const uint32_t *present,
		zcbor_encoder_t encoder,
		zcbor_state_t *state,
		const void *input);

#endif /* CBOR_ENCODE_H__ */
