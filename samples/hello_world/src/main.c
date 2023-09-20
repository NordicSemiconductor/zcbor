/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zcbor_decode.h>
#include <zcbor_encode.h>
#include <zcbor_common.h>
#include <stdio.h>

void main(void)
{
	uint8_t cbor_payload[15];
	bool success;
	struct zcbor_string decoded_string;

	/* Create zcbor state variable for encoding. */
	ZCBOR_STATE_E(encoding_state, 0, cbor_payload, sizeof(cbor_payload), 0);

	/* Encode a text string into the cbor_payload buffer */
	success = zcbor_tstr_put_lit(encoding_state, "Hello World");

	if (!success) {
		printf("Encoding failed: %d\r\n", zcbor_peek_error(encoding_state));
		return;
	}

	/* Create zcbor state variable for decoding. */
	ZCBOR_STATE_D(decoding_state, 0, cbor_payload, sizeof(cbor_payload), 1, 0);

	/* Decode the text string into the cbor_payload buffer */
	success = zcbor_tstr_decode(decoding_state, &decoded_string);

	if (!success) {
		printf("Decoding failed: %d\r\n", zcbor_peek_error(decoding_state));
		return;
	}

	printf("Decoded string: '%.*s'\r\n", (int)decoded_string.len, decoded_string.value);
}
