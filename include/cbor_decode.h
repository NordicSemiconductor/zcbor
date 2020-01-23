/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CDDL_CBOR_H__
#define CDDL_CBOR_H__
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


typedef enum
{
	CBOR_MAJOR_TYPE_PINT = 0,
	CBOR_MAJOR_TYPE_NINT = 1,
	CBOR_MAJOR_TYPE_BSTR = 2,
	CBOR_MAJOR_TYPE_TSTR = 3,
	CBOR_MAJOR_TYPE_LIST = 4,
	CBOR_MAJOR_TYPE_MAP  = 5,
	CBOR_MAJOR_TYPE_TAG  = 6,
	CBOR_MAJOR_TYPE_PRIM = 7,
} cbor_major_type_t;

typedef struct
{
	const uint8_t *value;
	size_t len;
} cbor_string_type_t;

#ifdef CDDL_CBOR_VERBOSE
#include <sys/printk.h>
#define cbor_decode_trace() (printk("*pp_payload: 0x%x, "\
			"**pp_payload: 0x%x, %s:%d\n",\
			(uint32_t)*pp_payload,\
			**pp_payload, __FILE__, __LINE__))
#define cbor_decode_print(...) printk(__VA_ARGS__)
#else
#define cbor_decode_trace()
#define cbor_decode_print(...)
#endif


typedef bool(decoder_t)(uint8_t const **, uint8_t const *const, size_t *const,
			void *, void *, void *);

bool intx32_decode(uint8_t const **const pp_payload, uint8_t const *const p_payload_end,
		size_t *const p_elem_count,
		int32_t *p_result, void *p_min_value, void *p_max_value);

bool uintx32_decode(uint8_t const **const pp_payload, uint8_t const *const p_payload_end,
		size_t *const p_elem_count,
		uint32_t *p_result, void *p_min_value, void *p_max_value);

bool strx_start_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		void *p_result, void *p_min_len,
		void *p_max_len);

bool strx_decode(uint8_t const **const pp_payload, uint8_t const *const p_payload_end,
		size_t *const p_elem_count,
		void *p_result, void *p_min_len, void *p_max_len);

bool list_start_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		size_t *p_result, size_t min_num,
		size_t max_num);

bool primx_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		uint8_t *p_result, void *p_min_result,
		void *p_max_result);

bool boolx_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		bool *p_result, void *p_min_result,
		void *p_max_result);

bool float_decode(uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		double *p_result, void *p_min_result,
		void *p_max_result);

bool any_decode(uint8_t const **const pp_payload, uint8_t const *const p_payload_end,
		size_t *const p_elem_count,
		void *p_result, void *p_min_result, void *p_max_result);

bool multi_decode(size_t min_decode, size_t max_decode, size_t *p_num_decode,
		decoder_t decoder, uint8_t const **const pp_payload,
		uint8_t const *const p_payload_end, size_t *const p_elem_count,
		void *p_result, void *p_min_result,
		void *p_max_result, size_t result_len);

#endif
