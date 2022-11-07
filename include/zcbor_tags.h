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

/** Values defined by RFC8949 via www.iana.org/assignments/cbor-tags/cbor-tags.xhtml */
enum zcbor_rfc8949_tag {
	ZCBOR_TAG_TIME_TSTR =            0, ///! text string       Standard date/time string
	ZCBOR_TAG_TIME_NUM =             1, ///! integer or float  Epoch-based date/time
	ZCBOR_TAG_UBIGNUM_BSTR =         2, ///! byte string       Unsigned bignum
	ZCBOR_TAG_BIGNUM_BSTR =          3, ///! byte string       Negative bignum
	ZCBOR_TAG_DECFRAC_ARR =          4, ///! array             Decimal fraction
	ZCBOR_TAG_BIGFLOAT_ARR =         5, ///! array             Bigfloat
	ZCBOR_TAG_2BASE64URL =          21, ///! (any)             Expected conversion to base64url encoding
	ZCBOR_TAG_2BASE64 =             22, ///! (any)             Expected conversion to base64 encoding
	ZCBOR_TAG_2BASE16 =             23, ///! (any)             Expected conversion to base16 encoding
	ZCBOR_TAG_BSTR =                24, ///! byte string       Encoded CBOR data item
	ZCBOR_TAG_URI_TSTR =            32, ///! text string       URI
	ZCBOR_TAG_BASE64URL_TSTR =      33, ///! text string       base64url
	ZCBOR_TAG_BASE64_TSTR =         34, ///! text string       base64
	ZCBOR_TAG_MIME_TSTR =           36, ///! text string       MIME message
	ZCBOR_TAG_CBOR =             55799, ///! (any)             Self-described CBOR
};

#ifdef __cplusplus
}
#endif

#endif /* ZCBOR_TAGS_H__ */
