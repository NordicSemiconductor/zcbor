/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "zcbor_common.h"

_Static_assert((sizeof(size_t) == sizeof(void *)),
	"This code needs size_t to be the same length as pointers.");

_Static_assert((sizeof(zcbor_state_t) >= sizeof(struct zcbor_state_constant)),
	"This code needs zcbor_state_t to be at least as large as zcbor_backups_t.");

bool zcbor_new_backup(zcbor_state_t *state, uint_fast32_t new_elem_count)
{
	if (!state->constant_state || ((state->constant_state->current_backup)
		>= state->constant_state->num_backups)) {
		ZCBOR_FAIL();
	}

	(state->constant_state->current_backup)++;

	/* use the backup at current_backup - 1, since otherwise, the 0th
	 * backup would be unused. */
	uint_fast32_t i = (state->constant_state->current_backup) - 1;

	memcpy(&state->constant_state->backup_list[i], state,
		sizeof(zcbor_state_t));

	state->elem_count = new_elem_count;

	return true;
}


bool zcbor_process_backup(zcbor_state_t *state, uint32_t flags,
		uint_fast32_t max_elem_count)
{
	const uint8_t *payload = state->payload;
	const uint_fast32_t elem_count = state->elem_count;

	if (!state->constant_state || (state->constant_state->current_backup == 0)) {
		ZCBOR_FAIL();
	}

	if (flags & ZCBOR_FLAG_RESTORE) {
		/* use the backup at current_backup - 1, since otherwise, the
		 * 0th backup would be unused. */
		uint_fast32_t i = state->constant_state->current_backup - 1;

		memcpy(state, &state->constant_state->backup_list[i],
			sizeof(zcbor_state_t));
	}

	if (flags & ZCBOR_FLAG_CONSUME) {
		state->constant_state->current_backup--;
	}

	if (elem_count > max_elem_count) {
		zcbor_print("elem_count: %" PRIuFAST32 " (expected max %" PRIuFAST32 ")\r\n",
			elem_count, max_elem_count);
		ZCBOR_FAIL();
	}

	if (flags & ZCBOR_FLAG_TRANSFER_PAYLOAD) {
		state->payload = payload;
	}

	return true;
}


bool zcbor_union_start_code(zcbor_state_t *state)
{
	if (!zcbor_new_backup(state, state->elem_count)) {
		ZCBOR_FAIL();
	}
	return true;
}


bool zcbor_union_elem_code(zcbor_state_t *state)
{
	if (!zcbor_process_backup(state, ZCBOR_FLAG_RESTORE, state->elem_count)) {
		ZCBOR_FAIL();
	}
	return true;
}

bool zcbor_union_end_code(zcbor_state_t *state)
{
	if (!zcbor_process_backup(state, ZCBOR_FLAG_CONSUME, state->elem_count)) {
		ZCBOR_FAIL();
	}
	return true;
}

void zcbor_new_state(zcbor_state_t *state_array, uint_fast32_t n_states,
		const uint8_t *payload, size_t payload_len, uint_fast32_t elem_count)
{
	state_array[0].payload = payload;
	state_array[0].payload_end = payload + payload_len;
	state_array[0].elem_count = elem_count;
	state_array[0].indefinite_length_array = false;
	state_array[0].constant_state = NULL;
	if (n_states > 2) {
		/* Use the last state as a struct zcbor_state_constant object. */
		state_array[0].constant_state = (struct zcbor_state_constant *)&state_array[n_states - 1];
		state_array[0].constant_state->backup_list = &state_array[1];
		state_array[0].constant_state->num_backups = n_states - 2;
		state_array[0].constant_state->current_backup = 0;
	}
}
