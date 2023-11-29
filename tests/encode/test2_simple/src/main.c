/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <pet_encode.h>
#include <zcbor_encode.h>


#define CONCAT_BYTE(a,b) a ## b

/* LIST() adds a start byte for a list with 'num' elements.
 * MAP() does the same, but for a map.
 * END adds an end byte for the list/map.
 *
 * With ZCBOR_CANONICAL, the start byte contains the list, so no end byte is
 * needed. Without ZCBOR_CANONICAL, the start byte is the same no matter
 * the number of elements, so it needs an explicit end byte.
 */
#ifndef ZCBOR_CANONICAL
#define LIST(num) 0x9F
#define MAP(num) 0xBF
#define END 0xFF,
#else
#define LIST(num) CONCAT_BYTE(0x8, num)
#define MAP(num) CONCAT_BYTE(0xA, num)
#define END
#endif


/* This test uses generated code to encode a 'Pet' instance. It populates the
 * generated struct, and runs the generated encoding function, then checks that
 * everything is correct.
 */
ZTEST(cbor_encode_test2, test_pet)
{
	struct Pet pet = {
		.names = {{.value = "foo", .len = 3}, {.value = "bar", .len = 3}},
		.names_count = 2,
		.birthday = {.value = (uint8_t[]){1,2,3,4,5,6,7,8}, .len = 8},
		.species_choice = Pet_species_dog_c
	};
	uint8_t exp_output[] = {
		LIST(3),
		LIST(2),
			0x63, 0x66, 0x6f, 0x6f, /* foo */
			0x63, 0x62, 0x61, 0x72, /* bar */
		END
		0x48, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x02, /* 2: dog */
		END
	};

	uint8_t output[25];
	size_t out_len;

	/* Check that encoding succeeded. */
	zassert_equal(ZCBOR_SUCCESS, cbor_encode_Pet(output, sizeof(output), &pet, &out_len), NULL);

	/* Check that the resulting length is correct. */
	zassert_equal(sizeof(exp_output), out_len, NULL);
	/* Check the payload contents. */
	zassert_mem_equal(exp_output, output, sizeof(exp_output), NULL);
}


/* This test uses the CBOR encoding library directly, i.e. no generated code.
 * It has no checking against a CDDL schema, but follows the "Pet" structure.
 * It sets up the zcbor_state_t variable.
 * It then makes a number of calls to functions in zcbor_encode.h and checks the
 * resulting payload agains the expected output.
 */
ZTEST(cbor_encode_test2, test_pet_raw)
{
	uint8_t payload[100] = {0};
	ZCBOR_STATE_E(state, 4, payload, sizeof(payload), 1);

	uint8_t exp_output[] = {
		LIST(3),
		LIST(2),
			0x65, 0x66, 0x69, 0x72, 0x73, 0x74, /* first */
			0x66, 0x73, 0x65, 0x63, 0x6f, 0x6e, 0x64, /* second */
		END
		0x48, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
		0x02, /* 2: dog */
		END
	};

	bool res = zcbor_list_start_encode(state, 0);
	zassert_true(res, NULL);

	res = res && zcbor_list_start_encode(state, 0);
	zassert_true(res, NULL);
	res = res && zcbor_tstr_put_lit(state, "first");
	zassert_true(res, NULL);
	res = res && zcbor_tstr_put_lit(state, "second");
	zassert_true(res, NULL);
	res = res && zcbor_list_end_encode(state, 0);
	zassert_true(res, NULL);
	uint8_t timestamp[8] = {1, 2, 3, 4, 5, 6, 7, 8};
	struct zcbor_string timestamp_str = {
		.value = timestamp,
		.len = sizeof(timestamp),
	};
	res = res && zcbor_bstr_encode(state, &timestamp_str);
	zassert_true(res, NULL);
	res = res && zcbor_uint32_put(state, 2 /* dog */);
	zassert_true(res, NULL);
	res = res && zcbor_list_end_encode(state, 0);

	/* Check that encoding succeeded. */
	zassert_true(res, NULL);
	/* Check that the resulting length is correct. */
	zassert_equal(sizeof(exp_output), state->payload - payload, "%d != %d\r\n",
		sizeof(exp_output), state->payload - payload);
	/* Check the payload contents. */
	zassert_mem_equal(exp_output, payload, sizeof(exp_output), NULL);
}

ZTEST_SUITE(cbor_encode_test2, NULL, NULL, NULL, NULL, NULL);
