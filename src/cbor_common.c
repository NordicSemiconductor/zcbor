/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "cbor_common.h"

_Static_assert((sizeof(size_t) == sizeof(void *)),
	"This code needs size_t to be the same length as pointers.");

_Static_assert((sizeof(cbor_state_t) >= sizeof(cbor_state_backups_t)),
	"This code needs cbor_state_t to be at least as large as cbor_backups_t.");

bool new_backup(cbor_state_t *state, uint32_t new_elem_count)
{
	if (!state->backups || ((state->backups->current_backup)
		>= state->backups->num_backups)) {
		FAIL();
	}

	(state->backups->current_backup)++;

	/* use the backup at current_backup - 1, since otherwise, the 0th
	 * backup would be unused. */
	uint32_t i = (state->backups->current_backup) - 1;

	memcpy(&state->backups->backup_list[i], state,
		sizeof(cbor_state_t));

	state->elem_count = new_elem_count;

	return true;
}


bool process_backup(cbor_state_t *state, uint32_t flags,
		uint32_t max_elem_count)
{
	const uint8_t *payload = state->payload;
	const uint32_t elem_count = state->elem_count;

	if (!state->backups || (state->backups->current_backup == 0)) {
		FAIL();
	}

	if (flags & FLAG_RESTORE) {
		/* use the backup at current_backup - 1, since otherwise, the
		 * 0th backup would be unused. */
		uint32_t i = state->backups->current_backup - 1;

		memcpy(state, &state->backups->backup_list[i],
			sizeof(cbor_state_t));
	}

	if (flags & FLAG_CONSUME) {
		state->backups->current_backup--;
	}

	if (elem_count > max_elem_count) {
		cbor_print("elem_count: %d (expected max %d)\r\n",
			elem_count, max_elem_count);
		FAIL();
	}

	if (flags & FLAG_TRANSFER_PAYLOAD) {
		state->payload = payload;
	}

	return true;
}


bool union_start_code(cbor_state_t *state)
{
	if (!new_backup(state, state->elem_count)) {
		FAIL();
	}
	return true;
}


bool union_elem_code(cbor_state_t *state)
{
	if (!process_backup(state, FLAG_RESTORE, state->elem_count)) {
		FAIL();
	}
	return true;
}

bool union_end_code(cbor_state_t *state)
{
	if (!process_backup(state, FLAG_CONSUME, state->elem_count)) {
		FAIL();
	}
	return true;
}

void new_state(cbor_state_t *state_array, uint32_t n_states,
		const uint8_t *payload, uint32_t payload_len, uint32_t elem_count)
{
	state_array[0].payload = payload;
	state_array[0].payload_end = payload + payload_len;
	state_array[0].elem_count = elem_count;
	state_array[0].backups = NULL;
	if (n_states > 2) {
		/* Use the last state as a cbor_state_backups_t object. */
		state_array[0].backups = (cbor_state_backups_t *)&state_array[n_states - 1];
		state_array[0].backups->backup_list = &state_array[1];
		state_array[0].backups->num_backups = n_states - 2;
		state_array[0].backups->current_backup = 0;
	}
}
