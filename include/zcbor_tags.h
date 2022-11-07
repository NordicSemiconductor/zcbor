/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZCBOR_TAGS_H__
#define ZCBOR_TAGS_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Values defined by RFCs via www.iana.org/assignments/cbor-tags/cbor-tags.xhtml */
enum zcbor_tag {
	ZCBOR_TAG_TIME_TSTR =            0, ///! text string       [RFC8949]  Standard date/time string
	ZCBOR_TAG_TIME_NUM =             1, ///! integer or float  [RFC8949]  Epoch-based date/time
	ZCBOR_TAG_UBIGNUM_BSTR =         2, ///! byte string       [RFC8949]  Unsigned bignum
	ZCBOR_TAG_BIGNUM_BSTR =          3, ///! byte string       [RFC8949]  Negative bignum
	ZCBOR_TAG_DECFRAC_ARR =          4, ///! array             [RFC8949]  Decimal fraction
	ZCBOR_TAG_BIGFLOAT_ARR =         5, ///! array             [RFC8949]  Bigfloat
	ZCBOR_TAG_2BASE64URL =          21, ///! (any)             [RFC8949]  Expected conversion to base64url encoding
	ZCBOR_TAG_2BASE64 =             22, ///! (any)             [RFC8949]  Expected conversion to base64 encoding
	ZCBOR_TAG_2BASE16 =             23, ///! (any)             [RFC8949]  Expected conversion to base16 encoding
	ZCBOR_TAG_BSTR =                24, ///! byte string       [RFC8949]  Encoded CBOR data item
	ZCBOR_TAG_URI_TSTR =            32, ///! text string       [RFC8949]  URI
	ZCBOR_TAG_BASE64URL_TSTR =      33, ///! text string       [RFC8949]  base64url
	ZCBOR_TAG_BASE64_TSTR =         34, ///! text string       [RFC8949]  base64
	ZCBOR_TAG_MIME_TSTR =           36, ///! text string       [RFC8949]  MIME message
	ZCBOR_TAG_CBOR =             55799, ///! (any)             [RFC8949]  Self-described CBOR
};

#ifdef __cplusplus
}
#endif

#endif /* ZCBOR_TAGS_H__ */
