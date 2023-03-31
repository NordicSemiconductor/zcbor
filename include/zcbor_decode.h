/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZCBOR_DECODE_H__
#define ZCBOR_DECODE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "zcbor_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/** The zcbor_decode library provides functions for decoding CBOR data elements.
 *
 * See The README for an introduction to CBOR, including the meaning of pint,
 * nint, bstr etc.
 */


/** See @ref zcbor_new_state() */
void zcbor_new_decode_state(zcbor_state_t *state_array, size_t n_states,
		const uint8_t *payload, size_t payload_len, size_t elem_count);

/** Convenience macro for declaring and initializing a decoding state with backups.
 *
 *  This gives you a state variable named @p name. The variable functions like
 *  a pointer.
 *
 *  @param[in]  name          The name of the new state variable.
 *  @param[in]  num_backups   The number of backup slots to keep in the state.
 *  @param[in]  payload       The payload to work on.
 *  @param[in]  payload_size  The size (in bytes) of @p payload.
 *  @param[in]  elem_count    The starting elem_count (typically 1).
 */
#define ZCBOR_STATE_D(name, num_backups, payload, payload_size, elem_count) \
zcbor_state_t name[((num_backups) + 2)]; \
do { \
	zcbor_new_decode_state(name, ZCBOR_ARRAY_SIZE(name), payload, payload_size, elem_count); \
} while(0)


/** The following applies to all _decode() functions listed directly below.
 *
 * @param[inout] state        The current state of the decoding.
 * @param[out]   result       Where to place the decoded value.
 * @param[in]    result_size  (if present) Size in bytes of the memory at @p result
 *
 * @retval true   If the value was decoded correctly.
 * @retval false  If the value has the wrong type, the payload overflowed, the
 *                element count was exhausted, or the value was larger than can
 *                fit in the result variable.
 *                Use zcbor_peek_error() to see the error code.
 */
bool zcbor_int32_decode(zcbor_state_t *state, int32_t *result); /* pint/nint */
bool zcbor_int64_decode(zcbor_state_t *state, int64_t *result); /* pint/nint */
bool zcbor_uint32_decode(zcbor_state_t *state, uint32_t *result); /* pint */
bool zcbor_uint64_decode(zcbor_state_t *state, uint64_t *result); /* pint */
bool zcbor_size_decode(zcbor_state_t *state, size_t *result); /* pint */
bool zcbor_int_decode(zcbor_state_t *state, void *result, size_t result_size); /* pint/nint */
bool zcbor_uint_decode(zcbor_state_t *state, void *result, size_t result_size); /* pint */
bool zcbor_bstr_decode(zcbor_state_t *state, struct zcbor_string *result); /* bstr */
bool zcbor_tstr_decode(zcbor_state_t *state, struct zcbor_string *result); /* tstr */
bool zcbor_tag_decode(zcbor_state_t *state, uint32_t *result);  /* CBOR tag */
bool zcbor_simple_decode(zcbor_state_t *state, uint8_t *result); /* CBOR simple value */
bool zcbor_bool_decode(zcbor_state_t *state, bool *result); /* boolean CBOR simple value */
bool zcbor_float16_decode(zcbor_state_t *state, float *result); /* IEEE754 float16 */
bool zcbor_float16_bytes_decode(zcbor_state_t *state, uint16_t *result); /* IEEE754 float16 raw bytes */
bool zcbor_float16_32_decode(zcbor_state_t *state, float *result); /* IEEE754 float16 or float32 */
bool zcbor_float32_decode(zcbor_state_t *state, float *result); /* IEEE754 float32 */
bool zcbor_float32_64_decode(zcbor_state_t *state, double *result); /* IEEE754 float32 or float64 */
bool zcbor_float64_decode(zcbor_state_t *state, double *result); /* IEEE754 float64 */
bool zcbor_float_decode(zcbor_state_t *state, double *result); /* IEEE754 float16, float32, or float64 */

/** The following applies to all _expect() functions listed directly below.
 *
 * @param[inout] state     The current state of the decoding.
 * @param[in]    expected  The expected value.
 *
 * @retval true   If the result was decoded correctly and has the expected value.
 * @retval false  If the decoding failed or the result doesn't have the
 *                expected value.
 *                Use zcbor_peek_error() to see the error code.
 */
bool zcbor_int32_expect(zcbor_state_t *state, int32_t expected); /* pint/nint */
bool zcbor_int64_expect(zcbor_state_t *state, int64_t expected); /* pint/nint */
bool zcbor_uint32_expect(zcbor_state_t *state, uint32_t expected); /* pint */
bool zcbor_uint64_expect(zcbor_state_t *state, uint64_t expected); /* pint */
bool zcbor_size_expect(zcbor_state_t *state, size_t expected); /* pint */
bool zcbor_bstr_expect(zcbor_state_t *state, struct zcbor_string *expected); /* bstr */
bool zcbor_tstr_expect(zcbor_state_t *state, struct zcbor_string *expected); /* tstr */
bool zcbor_tag_expect(zcbor_state_t *state, uint32_t expected); /* CBOR tag */
bool zcbor_simple_expect(zcbor_state_t *state, uint8_t expected); /* CBOR simple value */
bool zcbor_bool_expect(zcbor_state_t *state, bool expected); /* boolean CBOR simple value */
bool zcbor_nil_expect(zcbor_state_t *state, void *unused); /* 'nil' CBOR simple value */
bool zcbor_undefined_expect(zcbor_state_t *state, void *unused); /* 'undefined' CBOR simple value */
bool zcbor_float16_expect(zcbor_state_t *state, float expected); /* IEEE754 float16 */
bool zcbor_float16_bytes_expect(zcbor_state_t *state, uint16_t expected); /* IEEE754 float16 raw bytes */
bool zcbor_float16_32_expect(zcbor_state_t *state, float expected); /* IEEE754 float16 or float32 */
bool zcbor_float32_expect(zcbor_state_t *state, float expected); /* IEEE754 float32 */
bool zcbor_float32_64_expect(zcbor_state_t *state, double expected); /* IEEE754 float32 or float64 */
bool zcbor_float64_expect(zcbor_state_t *state, double expected); /* IEEE754 float64 */
bool zcbor_float_expect(zcbor_state_t *state, double expected); /* IEEE754 float16, float32, or float64 */

/** Consume and expect a pint/nint with a certain value, within a union.
 *
 * Calls @ref zcbor_union_elem_code then @ref zcbor_[u]int[32|64]_expect.
 */
bool zcbor_int32_expect_union(zcbor_state_t *state, int32_t expected);
bool zcbor_int64_expect_union(zcbor_state_t *state, int64_t expected);
bool zcbor_uint32_expect_union(zcbor_state_t *state, uint32_t expected);
bool zcbor_uint64_expect_union(zcbor_state_t *state, uint64_t expected);

/** Decode and consume a list/map header.
 *
 * The contents of the list can be decoded via subsequent function calls.
 * A state backup is created to keep track of the element count.
 * Call @ref zcbor_list_end_decode / @ref zcbor_map_end_decode when done
 * decoding the contents of the list/map
 *
 * @retval true   Header decoded correctly
 * @retval false  Header decoded incorrectly, or backup failed.
 */
bool zcbor_list_start_decode(zcbor_state_t *state);
bool zcbor_map_start_decode(zcbor_state_t *state);

/** Finalize decoding a list/map
 *
 * Check that the list/map had the correct number of elements, and restore the
 * previous element count from the backup.
 *
 * Use @ref zcbor_list_map_end_force_decode to forcibly consume the backup if
 * something has gone wrong.
 *
 * @retval true   Everything ok.
 * @retval false  Element count not correct.
 */
bool zcbor_list_end_decode(zcbor_state_t *state);
bool zcbor_map_end_decode(zcbor_state_t *state);
bool zcbor_list_map_end_force_decode(zcbor_state_t *state);

/** Skip a single element, regardless of type and value.
 *
 * This means if the element is a map or list, this function will recursively
 * skip all its contents.
 * This function will also skip any tags preceeding the element.
 *
 * @param[inout] state   The current state of the decoding.
 * @param[in]    unused  Unused parameter to maintain signature parity with
 *                       @ref zcbor_decoder_t.
 */
bool zcbor_any_skip(zcbor_state_t *state, void *unused);

/** Decode 0 or more elements with the same type and constraints.
 *
 * The decoded values will appear consecutively in the @p result array.
 *
 * The following is an example of decoding a list containing 3 INTS followed by
 * 0 to 2 bstrs:
 *
 * @code{c}
 *     uint32_t ints[3];
 *     struct zcbor_string bstrs[2];
 *     uint32_t num_decode;
 *     bool res;
 *
 *     res = zcbor_list_start_decode(state);
 *     res = res && zcbor_multi_decode(3, 3, &num_decode, zcbor_uint32_decode,
 *                  state, ints, sizeof(ints[0]));
 *     res = res && zcbor_multi_decode(0, 2, &num_decode, zcbor_bstr_decode,
 *                  state, bstrs, sizeof(bstrs[0]));
 *     res = res && zcbor_list_end_decode(state);
 *     // check res
 * @endcode
 *
 * The @ref zcbor_decoder_t type is designed to be compatible with all single-
 * value decoder functions in this library, e.g. @ref zcbor_uint32_decode,
 * @ref zcbor_tstr_expect, @ref zcbor_nil_expect, etc. For _expect() functions,
 * @p result will be used as a value instead of an array/pointer, so
 * @p result_len will determine how much the value changes for each call.
 * To decode the same value multiple times, use a @p result_len of 0.
 * This function can also be used with custom decoder functions, such as those
 * generated by the zcbor.py script, which for example decodes larger chunks of
 * the data at once.
 *
 * @param[in]  min_decode    The minimum acceptable number of elements.
 * @param[in]  max_decode    The maximum acceptable number of elements.
 * @param[out] num_decode    The actual number of elements decoded.
 * @param[in]  decoder       The decoder function to call under the hood. This
 *                           function will be called with the provided arguments
 *                           repeatedly until the function fails (returns false)
 *                           or until it has been called @p max_decode times.
 *                           The result pointer is moved @p result_len bytes for
 *                           each call to @p decoder, i.e. @p result refers to
 *                           an array of result variables.
 * @param[out] result        Where to place the decoded values. Must be an array
 *                           of at least @p max_decode elements.
 * @param[in]  result_len    The length of each result variable. Must be the
 *                           length of the individual elements of @p result.
 *
 * @retval true   If at least @p min_decode variables were correctly decoded.
 * @retval false  If @p decoder failed before having decoded @p min_decode
 *                values.
 */
bool zcbor_multi_decode(size_t min_decode, size_t max_decode, size_t *num_decode,
		zcbor_decoder_t decoder, zcbor_state_t *state, void *result,
		size_t result_len);

/** Attempt to decode a value that might not be present in the data.
 *
 * Works like @ref zcbor_multi_decode, with @p present as num_decode.
 * Will return true, even if the data is not present.
 *
 * @param[out] present  Whether or not the data was present and successfully decoded.
 * @param[in]  decoder  The decoder to attempt.
 * @param[out] result   The result, if present.
 *
 * @return Should always return true.
 */
bool zcbor_present_decode(bool *present,
		zcbor_decoder_t decoder,
		zcbor_state_t *state,
		void *result);


/** Supplementary string (bstr/tstr) decoding functions: */

/** Consume and expect a bstr/tstr with the value of the provided char/uint8_t array.
 *
 * @param[inout] state  The current state of the decoding.
 * @param[in]    str    The value to expect. A pointer to the string/array.
 *                      _term() uses strlen(), so @p str must be null-terminated.
 *                      _lit() uses sizeof()-1, so @p str must be a (null-terminated) string literal.
 *                      _arr() uses sizeof(), so @p str must be a uint8_t array (not null-terminated).
 * @param[in]    len    (if present) The length of the string pointed to by @p str
 */
bool zcbor_bstr_expect_ptr(zcbor_state_t *state, char const *ptr, size_t len);
bool zcbor_tstr_expect_ptr(zcbor_state_t *state, char const *ptr, size_t len);
bool zcbor_bstr_expect_term(zcbor_state_t *state, char const *str);
bool zcbor_tstr_expect_term(zcbor_state_t *state, char const *str);
#define zcbor_bstr_expect_lit(state, str) zcbor_bstr_expect_ptr(state, str, sizeof(str) - 1)
#define zcbor_tstr_expect_lit(state, str) zcbor_tstr_expect_ptr(state, str, sizeof(str) - 1)
#define zcbor_bstr_expect_arr(state, str) zcbor_bstr_expect_ptr(state, str, sizeof(str))
#define zcbor_tstr_expect_arr(state, str) zcbor_tstr_expect_ptr(state, str, sizeof(str))

/** Decode and consume a bstr header.
 *
 * The rest of the string can be decoded as CBOR.
 * A state backup is created to keep track of the element count.
 * Call @ref zcbor_bstr_end_decode when done decoding the contents of the bstr.
 *
 * @param[inout] state   The current state of the decoding.
 * @param[out]   result  The resulting string, for reference. The string should be decoded via
 *                       functions from this API since state is pointing to the start of the string,
 *                       not the end.
 *
 * @retval true   Header decoded correctly
 * @retval false  Header decoded incorrectly, or backup failed, or payload is not large enough
 *                to contain the contents of the string. Use @ref zcbor_bstr_start_decode_fragment
 *                for decoding fragmented payloads.
 */
bool zcbor_bstr_start_decode(zcbor_state_t *state, struct zcbor_string *result);

/** Finalize decoding a CBOR-encoded bstr.
 *
 * Restore element count from backup.
 */
bool zcbor_bstr_end_decode(zcbor_state_t *state);


/** Supplementary string (bstr/tstr) decoding functions for fragmented payloads: */

/** Start decoding a bstr/tstr, even if the payload contains only part of it.
 *
 * This must be followed by a call to @ref zcbor_update_state, which can be
 * followed by a call to @ref zcbor_next_fragment. Do not call this function
 * again on subsequent fragments of the same string.
 *
 * This consumes the remaining payload as long as it belongs to the string.
 */
bool zcbor_bstr_decode_fragment(zcbor_state_t *state, struct zcbor_string_fragment *result);
bool zcbor_tstr_decode_fragment(zcbor_state_t *state, struct zcbor_string_fragment *result);

/** Extract the next fragment of a string.
 *
 * Use this function to extract all but the first fragment.
 */
void zcbor_next_fragment(zcbor_state_t *state,
	struct zcbor_string_fragment *prev_fragment,
	struct zcbor_string_fragment *result);

/** Decode and consume a bstr header, assuming the payload does not contain the whole bstr.
 *
 * The rest of the string can be decoded as CBOR.
 * A state backup is created to keep track of the element count.
 * Call @ref zcbor_update_state followed by @ref zcbor_bstr_next_fragment when
 * the current payload has been exhausted.
 * Call @ref zcbor_bstr_end_decode when done decoding the contents of the bstr.
 */
bool zcbor_bstr_start_decode_fragment(zcbor_state_t *state,
	struct zcbor_string_fragment *result);

/** Start decoding the next fragment of a string.
 *
 * Use this function to extract all but the first fragment of a CBOR-encoded
 * bstr.
 */
void zcbor_bstr_next_fragment(zcbor_state_t *state,
	struct zcbor_string_fragment *prev_fragment,
	struct zcbor_string_fragment *result);

/** Can be used on any fragment to tell if it is the final fragment of the string. */
bool zcbor_is_last_fragment(const struct zcbor_string_fragment *fragment);

#ifdef __cplusplus
}
#endif

#endif /* ZCBOR_DECODE_H__ */
