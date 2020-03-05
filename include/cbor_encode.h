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
#include "cbor_common.h"


/** Encode a PINT/NINT into a int32_t.
 *
 * @param[inout] p_state        The current state of the decoding.
 * @param[out]   p_result       Where to place the encoded value.
 * @param[in]    p_min_value    The minimum acceptable value. This is checked
 *                              after decoding, and if the encoded value is
 *                              outside the range, the decoding will fail.
 *                              A NULL value here means there is no restriction.
 * @param[in]    p_max_value    The maximum acceptable value. This is checked
 *                              after decoding, and if the encoded value is
 *                              outside the range, the decoding will fail.
 *                              A NULL value here means there is no restriction.
 *
 * @retval true   If the value was encoded correctly.
 * @retval false  If the value has the wrong type, the payload overflowed, the
 *                element count was exhausted, the value was not within the
 *                acceptable range, or the value was larger than can fit in the
 *                result variable.
 */
bool intx32_encode(cbor_state_t * p_state, const int32_t *p_result, const int32_t *p_min_value, const int32_t *p_max_value);

/** Encode a PINT into a uint32_t.
 *
 * @details See @ref intx32_encode for information about parameters and return
 *          values.
 */
bool uintx32_encode(cbor_state_t * p_state, const uint32_t *p_result, const uint32_t *p_min_value, const uint32_t *p_max_value);

/** Encode a BSTR header, but leave p_payload ready to encode the string as CBOR.
 *
 * @param[in]  max_num  The maximum number of expected elements in the encoded
 *                      string.
 */
bool bstrx_cbor_start_encode(cbor_state_t *p_state, size_t max_num);

/** Finalize encoding a CBOR-encoded bstr.
 *
 * @param[in]  max_num  The maximum number of elements left unencoded.
 */
bool bstrx_cbor_end_encode(cbor_state_t *p_state, size_t max_elem_count);

/** Encode a BSTR, and move pp_payload to after the payload.
 *
 * @details See @ref intx32_encode for information about parameters and return
 *          values. For strings, the value refers to the length of the string.
 */
bool bstrx_encode(cbor_state_t * p_state, const cbor_string_type_t *p_result, const cbor_string_type_t *p_min, const size_t *p_max_len);

/** Encode a TSTR, and move pp_payload to after the payload.
 *
 * @details See @ref intx32_encode for information about parameters and return
 *          values. For strings, the value refers to the length of the string.
 */
bool tstrx_encode(cbor_state_t * p_state, const cbor_string_type_t *p_result, const cbor_string_type_t *p_min, const size_t *p_max_len);

/** Encode a LIST, but leave pp_payload pointing at the payload.
 *
 * @details This allocates a "state backup".
 *          See @ref intx32_encode for information about parameters and return
 *          values. For lists and maps, the value refers to the number of
 *          elements.
 */
bool list_start_encode(cbor_state_t * p_state, size_t min_num, size_t max_num);

/** Encode MAP, but leave pp_payload pointing at the payload.
 *
 * @details This allocates a "state backup".
 *          See @ref intx32_encode for information about parameters and return
 *          values. For lists and maps, the value refers to the number of
 *          elements.
 */
bool map_start_encode(cbor_state_t * p_state, size_t min_num, size_t max_num);

/** Encode end of a LIST. Do some checks and deallocate backup.
 *
 * @details See @ref intx32_encode for information about parameters and return
 *          values. For lists and maps, the value refers to the number of
 *          elements.
 */
bool list_end_encode(cbor_state_t * p_state, size_t min_num, size_t max_num);

/** Encode end of a MAP. Do some checks and deallocate backup.
 *
 * @details See @ref intx32_encode for information about parameters and return
 *          values. For lists and maps, the value refers to the number of
 *          elements.
 */
bool map_end_encode(cbor_state_t * p_state, size_t min_num, size_t max_num);

/** Encode a "nil" primitive value.
 *
 * @details All arguments except p_state are ignored and can be NULL.
 */
bool nilx_encode(cbor_state_t * p_state, const uint8_t *p_result, void *p_min_result, void *p_max_result);

/** Encode a boolean primitive value.
 *
 * @details See @ref intx32_encode for information about parameters and return
 *          values. The result is translated internally from the primitive
 *          values for true/false (20/21) to 0/1.
 */
bool boolx_encode(cbor_state_t * p_state, bool *p_result, const uint32_t *p_min_result, const uint32_t *p_max_result);

/** Encode a float
 *
 * @warning This function has not been tested, and likely doesn't work.
 *
 * @details See @ref intx32_encode for information about parameters and return
 *          values.
 */
bool float_encode(cbor_state_t * p_state, double *p_result, double *p_min_result, double *p_max_result);

/** Encode 0 or more elements with the same type and constraints.
 *
 * @details This must not necessarily encode all elements in a list. E.g. if
 *          the list contains 3 INTS between 0 and 100 followed by 0 to 2 BSTRs
 *          with length 8, that could be done with:
 *
 *          @code{c}
 *              uint32_t int_min = 0;
 *              uint32_t int_max = 100;
 *              size_t bstr_size = 8;
 *              uint32_t ints[3];
 *              cbor_string_type_t bstrs[2] = <initialize here>;
 *              bool res;
 *
 *              res = list_start_encode(p_state, 3, 5);
 *              // check res
 *              res = multi_encode(3, 3, &num_encode, uintx32_encode, p_state,
 *                           ints, &int_min, &int_max, 4);
 *              // check res
 *              res = multi_encode(0, 2, &num_encode, strx_encode, p_state,
 *                           bstrs, &bstr_size, &bstr_size,
 *                           sizeof(cbor_string_type_t));
 *              // check res
 *              res = list_end_encode(p_state, 3, 5);
 *              // check res
 *          @endcode
 *
 *          See @ref intx32_encode for information about the undocumented
 *          parameters.
 *
 * @param[in]  min_encode    The minimum acceptable number of elements.
 * @param[in]  max_encode    The maximum acceptable number of elements.
 * @param[in]  p_num_encode  The actual number of elements.
 * @param[in]  encoder       The encoder function to call under the hood. This
 *                           function will be called with the provided arguments
 *                           repeatedly until the function fails (returns false)
 *                           or until it has been called @p max_encode times.
 *                           p_result is moved @p result_len bytes for each call
 *                           to @p encoder, i.e. @p p_result refers to an array
 *                           of result variables.
 * @param[in]  p_result      Source of the encoded values. Must be an array
 *                           of length at least @p max_encode.
 * @param[in]  result_len    The length of the result variables. Must be the
 *                           length expected by the @p encoder.
 *
 * @retval true   If at least @p min_encode variables were correctly encoded.
 * @retval false  If @p encoder failed before having encoded @p min_encode
 *                values.
 */
bool multi_encode(size_t min_encode, size_t max_encode, const size_t *p_num_encode,
		cbor_encoder_t encoder, cbor_state_t * p_state, const void *p_result, const void *p_min_result, const void *p_max_result,
		size_t result_len);

#endif /* CBOR_ENCODE_H__ */
