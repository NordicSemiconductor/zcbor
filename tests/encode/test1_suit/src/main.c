/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include "manifest3_decode.h"
#include "manifest3_encode.h"
#include "zcbor_debug.h" // Enables use of print functions when debugging tests.

/* draft-ietf-suit-manifest-02 Example 0 */
uint8_t test_vector0_02[] = {
	0xa2, 0x01, 0x58, 0x54, 0xd2, 0x84, 0x43, 0xa1, 0x01,
	0x26, 0xa1, 0x04, 0x48, 0x74, 0x65, 0x73, 0x74, 0x20,
	0x6b, 0x65, 0x79, 0xf6, 0x58, 0x40, 0xeb, 0xec, 0xb6,
	0x6c, 0xbe, 0xcb, 0x19, 0xdc, 0xed, 0xac, 0xf8, 0x45,
	0x9c, 0x1a, 0x22, 0xa1, 0x45, 0x37, 0x81, 0xba, 0x98,
	0xd8, 0xff, 0xb9, 0xd4, 0xe2, 0x91, 0x2f, 0x29, 0xd2,
	0x3b, 0xac, 0x5a, 0xe3, 0xd5, 0x1f, 0x1f, 0xf0, 0xc1,
	0xb1, 0xdf, 0x05, 0xe2, 0x07, 0xca, 0x17, 0x48, 0x3a,
	0x57, 0xed, 0xe9, 0x14, 0xcf, 0x82, 0x6b, 0x73, 0x59,
	0x91, 0x37, 0x88, 0x1c, 0x83, 0x64, 0xc8, 0x02, 0x58,
	0x51, 0xa4, 0x01, 0x01, 0x02, 0x01, 0x03, 0x58, 0x40,
	0xa2, 0x02, 0x4c, 0x81, 0x82, 0x45, 0x46, 0x6c, 0x61,
	0x73, 0x68, 0x43, 0x00, 0x34, 0x01, 0x04, 0x58, 0x2e,
	0x82, 0x13, 0xa2, 0x0b, 0x58, 0x24, 0x82, 0x02, 0x58,
	0x20, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x01,
	0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc,
	0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0x0c, 0x19, 0x87,
	0xd0, 0x0c, 0x47, 0x86, 0x0c, 0x00, 0x03, 0xf6, 0x17,
	0xf6
};

/* draft-ietf-suit-manifest-03 Example 0 */
uint8_t test_vector0[] = {
	0xa1, 0x02, 0x58, 0x6c, 0xa5, 0x01, 0x01, 0x02,
	0x01, 0x03, 0x58, 0x5a, 0xa2, 0x02, 0x44, 0x81,
	0x81, 0x41, 0x00, 0x04, 0x58, 0x50, 0x86, 0x01,
	0x50, 0xfa, 0x6b, 0x4a, 0x53, 0xd5, 0xad, 0x5f,
	0xdf, 0xbe, 0x9d, 0xe6, 0x63, 0xe4, 0xd4, 0x1f,
	0xfe, 0x02, 0x50, 0x14, 0x92, 0xaf, 0x14, 0x25,
	0x69, 0x5e, 0x48, 0xbf, 0x42, 0x9b, 0x2d, 0x51,
	0xf2, 0xab, 0x45, 0x14, 0xa2, 0x0b, 0x82, 0x02,
	0x58, 0x20, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
	0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd,
	0xee, 0xff, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
	0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54,
	0x32, 0x10, 0x0c, 0x19, 0x87, 0xd0, 0x0a, 0x43,
	0x82, 0x03, 0xf6, 0x0c, 0x43, 0x82, 0x17, 0xf6
};

/* draft-ietf-suit-manifest-03 Example 1 */
uint8_t test_vector1[] = {
	0xa1, 0x03, 0x58, 0x8a, 0xa4, 0x01, 0x01, 0x02,
	0x02, 0x03, 0x58, 0x5a, 0xa2, 0x02, 0x44, 0x81,
	0x81, 0x41, 0x00, 0x04, 0x58, 0x50, 0x86, 0x01,
	0x50, 0xfa, 0x6b, 0x4a, 0x53, 0xd5, 0xad, 0x5f,
	0xdf, 0xbe, 0x9d, 0xe6, 0x63, 0xe4, 0xd4, 0x1f,
	0xfe, 0x02, 0x50, 0x14, 0x92, 0xaf, 0x14, 0x25,
	0x69, 0x5e, 0x48, 0xbf, 0x42, 0x9b, 0x2d, 0x51,
	0xf2, 0xab, 0x45, 0x14, 0xa2, 0x0b, 0x82, 0x02,
	0x58, 0x20, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
	0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd,
	0xee, 0xff, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
	0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54,
	0x32, 0x10, 0x0c, 0x19, 0x87, 0xd0, 0x09, 0x58,
	0x25, 0x86, 0x13, 0xa1, 0x06, 0x78, 0x1b, 0x68,
	0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x65, 0x78,
	0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f,
	0x6d, 0x2f, 0x66, 0x69, 0x6c, 0x65, 0x2e, 0x62,
	0x69, 0x6e, 0x15, 0xf6, 0x03, 0xf6
};

/* draft-ietf-suit-manifest-03 Example 2 */
uint8_t test_vector2[] = {
	0xa2, 0x02, 0x58, 0x70, 0x81, 0xd2, 0x84, 0x43,
	0xa1, 0x01, 0x26, 0xa0, 0x58, 0x24, 0x82, 0x02,
	0x58, 0x20, 0x70, 0xcf, 0x2a, 0x4f, 0xed, 0x64,
	0x06, 0x58, 0xad, 0xa6, 0xff, 0x33, 0xb5, 0x9a,
	0xf1, 0x92, 0xca, 0x22, 0xb4, 0x14, 0x2e, 0x9a,
	0xe9, 0xd8, 0xd9, 0xb0, 0x5f, 0x2b, 0x5a, 0x11,
	0x8c, 0xf3, 0x58, 0x40, 0xf6, 0xc9, 0x56, 0x81,
	0xef, 0x42, 0x98, 0x12, 0x88, 0xe1, 0x10, 0x04,
	0xa4, 0xb7, 0x2b, 0xe8, 0x0a, 0x37, 0x4b, 0xe1,
	0x3e, 0xfc, 0xcf, 0x5e, 0xc9, 0x4f, 0xa1, 0xad,
	0x2c, 0xa7, 0xd5, 0x51, 0x0d, 0x5f, 0xf4, 0x3c,
	0xea, 0xc6, 0x0e, 0x7d, 0xd3, 0x2d, 0x36, 0x14,
	0xbd, 0x03, 0x50, 0x76, 0x8f, 0x98, 0x5e, 0xff,
	0x8b, 0xa9, 0x93, 0x36, 0x25, 0xd2, 0x06, 0x28,
	0x6c, 0xf9, 0x83, 0x03, 0x58, 0x94, 0x01, 0x01,
	0x02, 0x03, 0x03, 0x58, 0x5a, 0xa2, 0x02, 0x44,
	0x81, 0x81, 0x41, 0x00, 0x04, 0x58, 0x50, 0x86,
	0x01, 0x50, 0xfa, 0x6b, 0x4a, 0x53, 0xd5, 0xad,
	0x5f, 0xdf, 0xbe, 0x9d, 0xe6, 0x63, 0xe4, 0xd4,
	0x1f, 0xfe, 0x02, 0x50, 0x14, 0x92, 0xaf, 0x14,
	0x25, 0x69, 0x5e, 0x48, 0xbf, 0x42, 0x9b, 0x2d,
	0x51, 0xf2, 0xab, 0x45, 0x14, 0xa2, 0x0b, 0x82,
	0x02, 0x20, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
	0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd,
	0xee, 0xff, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
	0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54,
	0x32, 0x10, 0x0c, 0x19, 0x87, 0xd0, 0x09, 0x58,
	0x25, 0x86, 0x13, 0xa1, 0x06, 0x78, 0x1b, 0x68,
	0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x65, 0x78,
	0x61, 0x6d, 0x70, 0x6c, 0x2e, 0x63, 0x6f, 0x6d,
	0x2f, 0x66, 0x69, 0x6c, 0x65, 0x2e, 0x62, 0x69,
	0x6e, 0x15, 0xf6, 0x03, 0xf6, 0x0a, 0x43, 0x82,
	0x03, 0xf6, 0x0c, 0x43, 0x82, 0x17, 0xf6,
};

/* draft-ietf-suit-manifest-03 Example 3 */
uint8_t test_vector3[] = {
	0xa2, 0x02, 0x58, 0x70, 0x81, 0xd2, 0x84, 0x43,
	0xa1, 0x01, 0x26, 0xa0, 0x58, 0x24, 0x82, 0x02,
	0x58, 0x20, 0xbb, 0x00, 0x8f, 0x57, 0xfd, 0x1b,
	0xab, 0xff, 0x8c, 0xc4, 0x32, 0xd1, 0x8c, 0x4c,
	0x9c, 0xfc, 0x69, 0xd7, 0xe8, 0xab, 0x76, 0xb0,
	0x7c, 0xc9, 0x10, 0xc6, 0xd0, 0x3e, 0xc5, 0x98,
	0xba, 0xab, 0x58, 0x40, 0x9e, 0x98, 0xc5, 0x8f,
	0xcd, 0x82, 0x66, 0x84, 0x43, 0xa0, 0x24, 0x9f,
	0xa5, 0xea, 0xb1, 0x04, 0x74, 0xa0, 0x99, 0x57,
	0x2d, 0xfb, 0x31, 0xc0, 0xd2, 0xad, 0xf7, 0x50,
	0xf5, 0x7c, 0x49, 0x87, 0xd4, 0x84, 0xba, 0xdf,
	0x85, 0x24, 0xa2, 0x0a, 0x9e, 0x92, 0xc4, 0x59,
	0x96, 0x98, 0xeb, 0x69, 0x62, 0x54, 0xd4, 0xc0,
	0xf7, 0x79, 0x47, 0xc8, 0xaf, 0x35, 0x3b, 0x54,
	0x46, 0x00, 0xea, 0x11, 0x03, 0x58, 0xd6, 0xa7,
	0x01, 0x01, 0x02, 0x04, 0x03, 0x58, 0x5f, 0xa2,
	0x02, 0x47, 0x82, 0x81, 0x41, 0x00, 0x81, 0x41,
	0x01, 0x04, 0x58, 0x52, 0x88, 0x0c, 0x00, 0x01,
	0x50, 0xfa, 0x6b, 0x4a, 0x53, 0xd5, 0xad, 0x5f,
	0xdf, 0xbe, 0x9d, 0xe6, 0x63, 0xe4, 0xd4, 0x1f,
	0xfe, 0x02, 0x50, 0x14, 0x92, 0xaf, 0x14, 0x25,
	0x69, 0x5e, 0x48, 0xbf, 0x42, 0x9b, 0x2d, 0x51,
	0xf2, 0xab, 0x45, 0x14, 0xa2, 0x0b, 0x82, 0x02,
	0x58, 0x20, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
	0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd,
	0xee, 0xff, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab,
	0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54,
	0x32, 0x10, 0x0c, 0x19, 0x87, 0xd0, 0x09, 0x58,
	0x27, 0x88, 0x0c, 0x00, 0x13, 0xa1, 0x06, 0x78,
	0x1b, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f,
	0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e,
	0x63, 0x6f, 0x6d, 0x2f, 0x66, 0x69, 0x6c, 0x65,
	0x2e, 0x62, 0x69, 0x6e, 0x15, 0xf6, 0x03, 0xf6,
	0x0a, 0x45, 0x84, 0x0c, 0x00, 0x03, 0xf6, 0x0b,
	0x58, 0x34, 0x88, 0x0c, 0x01, 0x14, 0xa3, 0x0a,
	0x00, 0x0b, 0x82, 0x02, 0x58, 0x20, 0x00, 0x11,
	0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
	0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x01, 0x23,
	0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc,
	0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0x0c, 0x19,
	0x87, 0xd0, 0x16, 0xf6, 0x03, 0xf6, 0x0c, 0x45,
	0x84, 0x0c, 0x01, 0x17, 0xf6,
};

/* draft-ietf-suit-manifest-03 Example 4 */
uint8_t test_vector4[] = {
	0xa2, 0x01, 0x58, 0x70, 0x81, 0xd2, 0x84, 0x43,
	0xa1, 0x01, 0x26, 0xa0, 0x58, 0x24, 0x82, 0x02,
	0x58, 0x20, 0xb9, 0x73, 0xe2, 0x42, 0x4d, 0x03,
	0xde, 0x20, 0xc5, 0x9c, 0xb7, 0x02,
	0x60, 0x7a, 0x83, 0x79, 0x6d, 0xd4, 0x65, 0x67,
	0x41, 0x15, 0xae, 0x84, 0xb3, 0xc2, 0xc4, 0x72,
	0x79, 0x4d, 0xbb, 0x8c, 0x58, 0x40, 0xbe, 0x0a,
	0xe3, 0xd3, 0x60, 0xe4, 0x6d, 0xd0,
	0x7f, 0x02, 0x54, 0x7f, 0xf1, 0x9e, 0x4a, 0x15,
	0x57, 0xb7, 0xbf, 0xce, 0x40, 0x17, 0x18, 0xad,
	0xe8, 0x20, 0x09, 0x18, 0xf1, 0x91, 0xa5, 0x0d,
	0xca, 0x84, 0x14, 0x87, 0x04, 0xf7,
	0x6d, 0x97, 0xa8, 0xc2, 0x39, 0x61, 0x51, 0x14,
	0xea, 0xb0, 0x61, 0x7e, 0x9f, 0xc9, 0xd4, 0xfa,
	0xea, 0xc1, 0x57, 0x2e, 0x7c, 0xae, 0x61, 0xe6,
	0x60, 0xc1, 0x02, 0x58, 0xd8, 0xa7,
	0x01, 0x01, 0x02, 0x05, 0x03, 0x58, 0x5f, 0xa2,
	0x02, 0x47, 0x82, 0x81, 0x41, 0x00, 0x81, 0x41,
	0x01, 0x04, 0x58, 0x52, 0x88, 0x0c, 0x00, 0x01,
	0x50, 0xfa, 0x6b, 0x4a, 0x53, 0xd5,
	0xad, 0x5f, 0xdf, 0xbe, 0x9d, 0xe6, 0x63, 0xe4,
	0xd4, 0x1f, 0xfe, 0x02, 0x50, 0x14, 0x92, 0xaf,
	0x14, 0x25, 0x69, 0x5e, 0x48, 0xbf, 0x42, 0x9b,
	0x2d, 0x51, 0xf2, 0xab, 0x45, 0x14,
	0xa2, 0x0b, 0x82, 0x02, 0x58, 0x20, 0x00, 0x11,
	0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
	0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x01, 0x23,
	0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
	0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
	0x0c, 0x19, 0x87, 0xd0, 0x09, 0x58, 0x27, 0x88,
	0x0c, 0x00, 0x13, 0xa1, 0x06, 0x78, 0x1b, 0x68,
	0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f,
	0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e,
	0x63, 0x6f, 0x6d, 0x2f, 0x66, 0x69, 0x6c, 0x65,
	0x2e, 0x62, 0x69, 0x6e, 0x15, 0xf6, 0x03, 0xf6,
	0x0a, 0x45, 0x84, 0x0c, 0x00, 0x03,
	0xf6, 0x0b, 0x58, 0x36, 0x88, 0x0c, 0x01, 0x14,
	0xa4, 0x08, 0x01, 0x0a, 0x00, 0x0b, 0x82, 0x02,
	0x58, 0x20, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
	0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
	0xcc, 0xdd, 0xee, 0xff, 0x01, 0x23, 0x45, 0x67,
	0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98,
	0x76, 0x54, 0x32, 0x10, 0x0c, 0x19, 0x87, 0xd0,
	0x16, 0xf6, 0x03, 0xf6, 0x0c, 0x45,
	0x84, 0x0c, 0x01, 0x17, 0xf6
};

const uint8_t *test_vector[] = {
	test_vector0_02,
	// test_vector0,
	// test_vector1,
	// test_vector2,
	// test_vector3,
	// test_vector4,
};

uint32_t test_vector_len[] = {
	sizeof(test_vector0_02),
	// sizeof(test_vector0),
	// sizeof(test_vector1),
	// sizeof(test_vector2),
	// sizeof(test_vector3),
	// sizeof(test_vector4),
};

uint32_t num_test_vectors = sizeof(test_vector)/sizeof(uint8_t*);


static uint8_t output[2000];

void test_command_sequence(struct zcbor_string *sequence_str,
			bool present, char *name)
{
	struct SUIT_Command_Sequence sequence1;
	struct zcbor_string *run_seq;
	bool run_seq_present;
	struct zcbor_string *try_each1;
	bool try_each1_present;
	struct zcbor_string *try_each2;
	bool try_each2_present;
	size_t out_len;

	if (!present) {
		return;
	}

	zcbor_print("\r\ntest %s\r\n", name);

	memset(&sequence1, 0, sizeof(sequence1));
	int res = cbor_decode_SUIT_Command_Sequence(sequence_str->value,
		sequence_str->len,
		&sequence1, NULL);
	zassert_equal(ZCBOR_SUCCESS, res, NULL);


	res = cbor_encode_SUIT_Command_Sequence(output,
		sizeof(output),
		&sequence1, &out_len);
	zassert_equal(ZCBOR_SUCCESS, res, NULL);
	zassert_equal(sequence_str->len, out_len, "%d != %d\r\n", sequence_str->len, out_len);
	zassert_mem_equal(sequence_str->value,
		output,
		sequence_str->len,
		NULL);

	for (uint32_t i = 0; i < sequence1._SUIT_Command_Sequence_union_count; i++) {
		struct SUIT_Directive_ *directive;
		bool directive_present;
		directive = &sequence1
			._SUIT_Command_Sequence_union[i]
			._SUIT_Command_Sequence_union__SUIT_Directive;
		directive_present = sequence1
			._SUIT_Command_Sequence_union[i]
			._SUIT_Command_Sequence_union_choice
			== _SUIT_Command_Sequence_union__SUIT_Directive;

		run_seq = &directive
			->_SUIT_Directive___suit_directive_run_sequence_SUIT_Command_Sequence_bstr;
		run_seq_present = directive_present
			&& directive
			->_SUIT_Directive_choice
			== _SUIT_Directive___suit_directive_run_sequence;
		test_command_sequence(run_seq, run_seq_present, "run_seq");

		for (uint32_t j = 0; j < directive
			->_SUIT_Directive___suit_directive_try_each__SUIT_Directive_Try_Each_Argument
			._SUIT_Directive_Try_Each_Argument_SUIT_Command_Sequence_bstr_count; j++) {

			try_each1 = &directive
				->_SUIT_Directive___suit_directive_try_each__SUIT_Directive_Try_Each_Argument
				._SUIT_Directive_Try_Each_Argument_SUIT_Command_Sequence_bstr[j];
			try_each1_present = directive_present
				&& directive->_SUIT_Directive_choice
				== _SUIT_Directive___suit_directive_try_each;
			test_command_sequence(try_each1, try_each1_present, "try_each1");
		}

		try_each2 = &directive
			->_SUIT_Directive___suit_directive_try_each__SUIT_Directive_Try_Each_Argument
			._SUIT_Directive_Try_Each_Argument_union_SUIT_Command_Sequence_bstr;
		try_each2_present = directive_present
			&& directive->_SUIT_Directive_choice
			== _SUIT_Directive___suit_directive_try_each
			&& directive
			->_SUIT_Directive___suit_directive_try_each__SUIT_Directive_Try_Each_Argument
			._SUIT_Directive_Try_Each_Argument_union_choice
			== _SUIT_Directive_Try_Each_Argument_union_SUIT_Command_Sequence_bstr;
		test_command_sequence(try_each2, try_each2_present, "try_each2");

	}
}

void test_manifest(const uint8_t *input, uint32_t len)
{
	struct SUIT_Outer_Wrapper outerwrapper1 = {0};
	struct SUIT_Manifest *manifest;
	// struct SUIT_Component_Identifier *component;
	struct zcbor_string *dependency1;
	bool dependency1_present;
	struct zcbor_string *fetch1;
	bool fetch1_present;
	struct zcbor_string *install1;
	bool install1_present;
	struct zcbor_string *common_seq;
	bool common_seq_present;
	struct zcbor_string *dependency2;
	bool dependency2_present;
	struct zcbor_string *fetch2;
	bool fetch2_present;
	struct zcbor_string *install2;
	bool install2_present;
	struct zcbor_string *validate;
	bool validate_present;
	struct zcbor_string *load;
	bool load_present;
	struct zcbor_string *run;
	bool run_present;
	int res;
	size_t out_len;

	zcbor_print("test_vector at: 0x%zx\r\n", (size_t)input);
	zcbor_print("test_vector end at: 0x%zx\r\n",
				((size_t)input) + len);
	res = cbor_decode_SUIT_Outer_Wrapper(input, len, &outerwrapper1, NULL);
	zassert_equal(ZCBOR_SUCCESS, res, "top-level decoding failed.");

	dependency1 = &outerwrapper1
		._SUIT_Outer_Wrapper_suit_dependency_resolution
		._SUIT_Outer_Wrapper_suit_dependency_resolution;
	dependency1_present = outerwrapper1
		._SUIT_Outer_Wrapper_suit_dependency_resolution_present;
	test_command_sequence(dependency1, dependency1_present, "dependency1");

	fetch1 = &outerwrapper1
		._SUIT_Outer_Wrapper_suit_payload_fetch
		._SUIT_Outer_Wrapper_suit_payload_fetch;
	fetch1_present = outerwrapper1
		._SUIT_Outer_Wrapper_suit_payload_fetch_present;
	test_command_sequence(fetch1, fetch1_present, "fetch1");

	install1 = &outerwrapper1
		._SUIT_Outer_Wrapper_suit_install
		._SUIT_Outer_Wrapper_suit_install;
	install1_present = outerwrapper1
		._SUIT_Outer_Wrapper_suit_install_present;
	test_command_sequence(install1, install1_present, "install1");

	manifest = &outerwrapper1
		._SUIT_Outer_Wrapper__SUIT_Manifest_Wrapped
		._SUIT_Manifest_Wrapped_suit_manifest_cbor;

	common_seq = &manifest
		->_SUIT_Manifest_suit_common
		._SUIT_Manifest_suit_common_cbor
		._SUIT_Common_suit_common_sequence
		._SUIT_Common_suit_common_sequence;
	common_seq_present = manifest
		->_SUIT_Manifest_suit_common_present
		&& manifest
		->_SUIT_Manifest_suit_common
		._SUIT_Manifest_suit_common_cbor
		._SUIT_Common_suit_common_sequence_present;
	test_command_sequence(common_seq, common_seq_present, "common_seq");

	dependency2 = &manifest
		->_SUIT_Manifest_suit_dependency_resolution
		._SUIT_Manifest_suit_dependency_resolution_SUIT_Command_Sequence_bstr;
	dependency2_present = manifest
		->_SUIT_Manifest_suit_dependency_resolution_present
		&& manifest
		->_SUIT_Manifest_suit_dependency_resolution
		._SUIT_Manifest_suit_dependency_resolution_choice ==
		_SUIT_Manifest_suit_dependency_resolution_SUIT_Command_Sequence_bstr;
	test_command_sequence(dependency2, dependency2_present, "dependency2");

	fetch2 = &manifest
		->_SUIT_Manifest_suit_payload_fetch
		._SUIT_Manifest_suit_payload_fetch_SUIT_Command_Sequence_bstr;
	fetch2_present = manifest
		->_SUIT_Manifest_suit_payload_fetch_present
		&& manifest
		->_SUIT_Manifest_suit_payload_fetch
		._SUIT_Manifest_suit_payload_fetch_choice ==
		_SUIT_Manifest_suit_payload_fetch_SUIT_Command_Sequence_bstr;
	test_command_sequence(fetch2, fetch2_present, "fetch2");

	install2 = &manifest
		->_SUIT_Manifest_suit_install
		._SUIT_Manifest_suit_install_SUIT_Command_Sequence_bstr;
	install2_present = manifest
		->_SUIT_Manifest_suit_install_present
		&& manifest
		->_SUIT_Manifest_suit_install
		._SUIT_Manifest_suit_install_choice ==
		_SUIT_Manifest_suit_install_SUIT_Command_Sequence_bstr;
	test_command_sequence(install2, install2_present, "install2");

	validate = &manifest
		->_SUIT_Manifest_suit_validate
		._SUIT_Manifest_suit_validate;
	validate_present = manifest
		->_SUIT_Manifest_suit_validate_present;
	test_command_sequence(validate, validate_present, "validate");

	load = &manifest
		->_SUIT_Manifest_suit_load
		._SUIT_Manifest_suit_load;
	load_present = manifest
		->_SUIT_Manifest_suit_load_present;
	test_command_sequence(load, load_present, "load");

	run = &manifest
		->_SUIT_Manifest_suit_run
		._SUIT_Manifest_suit_run;
	run_present = manifest
		->_SUIT_Manifest_suit_run_present;
	test_command_sequence(run, run_present, "run");

	res = cbor_encode_SUIT_Outer_Wrapper(output,
					sizeof(output), &outerwrapper1, &out_len);
	zassert_equal(ZCBOR_SUCCESS, res, "top-level encoding failed.");
	zassert_equal(len, out_len, NULL);
	zassert_mem_equal(input, output, len, NULL);
}

void test_suit(void)
{
	for(uint32_t i = 0; i < num_test_vectors; i++) {
		test_manifest(test_vector[i], test_vector_len[i]);
	}
}

void test_main(void)
{
	ztest_test_suite(cbor_encode_test1,
			 ztest_unit_test(test_suit)
	);
	ztest_run_test_suite(cbor_encode_test1);
}
