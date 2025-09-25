/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Generated using zcbor version 0.9.99
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "zcbor_encode.h"
#include "pet_encode.h"
#include "zcbor_print.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif

#define ZCBOR_CUSTOM_CAST_FP(func) _Generic((func), \
	bool(*)(zcbor_state_t *, const struct Pet *): ((zcbor_encoder_t *)func), \
	default: ZCBOR_CAST_FP(func))

#define log_result(state, result, func) do { \
	if (!result) { \
		zcbor_trace_file(state); \
		zcbor_log("%s error: %s\r\n", func, zcbor_error_str(zcbor_peek_error(state))); \
	} else { \
		zcbor_log("%s success\r\n", func); \
	} \
} while(0)

static bool encode_Pet(zcbor_state_t *state, const struct Pet *input);


static bool encode_Pet(
		zcbor_state_t *state, const struct Pet *input)
{
	zcbor_log("%s\r\n", __func__);

	bool res = (((zcbor_list_start_encode(state, 3) && ((((zcbor_list_start_encode(state, 3) && ((zcbor_multi_encode_minmax(PET_NAME_NAMES_MIN_REP, PET_NAME_NAMES_MAX_REP, &(*input).names_count, ZCBOR_CUSTOM_CAST_FP(zcbor_tstr_encode), state, (*&(*input).names), sizeof(struct zcbor_string))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 3)))
	&& (((((((*input).birthday.len == TIMESTAMP_SIZE)) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))) || (zcbor_error(state, ZCBOR_ERR_WRONG_RANGE), false))
	&& (zcbor_bstr_encode(state, (&(*input).birthday))))
	&& ((((*input).species_choice == Pet_species_cat_c) ? ((zcbor_uint8_put(state, (PET_SPECIES_CAT_VAL))))
	: (((*input).species_choice == Pet_species_dog_c) ? ((zcbor_uint8_put(state, (PET_SPECIES_DOG_VAL))))
	: (((*input).species_choice == Pet_species_other_c) ? ((zcbor_uint8_put(state, (PET_SPECIES_OTHER_VAL))))
	: false))))) || (zcbor_list_map_end_force_encode(state), false)) && zcbor_list_end_encode(state, 3))));

	if (false) {
		/* For testing that the types of the arguments are correct.
		 * A compiler error here means a bug in zcbor.
		 */
		zcbor_tstr_encode(state, (*&(*input).names));
	}

	log_result(state, res, __func__);
	return res;
}



int cbor_encode_Pet(
		uint8_t *payload, size_t payload_len,
		const struct Pet *input,
		size_t *payload_len_out)
{
	zcbor_state_t states[4];

	if (false) {
		/* For testing that the types of the arguments are correct.
		 * A compiler error here means a bug in zcbor.
		 */
		encode_Pet(states, input);
	}

	return zcbor_entry_function(payload, payload_len, (void *)input, payload_len_out, states,
		(zcbor_decoder_t *)ZCBOR_CUSTOM_CAST_FP(encode_Pet), sizeof(states) / sizeof(zcbor_state_t), 1);
}
