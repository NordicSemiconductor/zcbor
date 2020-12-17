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

bool new_backup(cbor_state_t *p_state, size_t new_elem_count)
{
	if ((p_state->p_backups->current_backup + 1)
		>= p_state->p_backups->num_backups) {
		FAIL();
	}

	size_t i = ++(p_state->p_backups->current_backup);
	memcpy(&p_state->p_backups->p_backup_list[i], p_state,
		sizeof(cbor_state_t));

	p_state->elem_count = new_elem_count;

	return true;
}


bool restore_backup(cbor_state_t *p_state, uint32_t flags,
		size_t max_elem_count)
{
	const uint8_t *p_payload = p_state->p_payload;
	const size_t elem_count = p_state->elem_count;

	if (p_state->p_backups->current_backup == 0) {
		FAIL();
	}

	if (flags & FLAG_RESTORE) {
		size_t i = p_state->p_backups->current_backup;

		memcpy(p_state, &p_state->p_backups->p_backup_list[i],
			sizeof(cbor_state_t));
	}

	if (flags & FLAG_DISCARD) {
		p_state->p_backups->current_backup--;
	}

	if (elem_count > max_elem_count) {
		cbor_print("elem_count: %d (expected max %d)\r\n",
			elem_count, max_elem_count);
		FAIL();
	}

	if (flags & FLAG_TRANSFER_PAYLOAD) {
		p_state->p_payload = p_payload;
	}

	return true;
}


bool union_start_code(cbor_state_t *p_state)
{
	if (!new_backup(p_state, p_state->elem_count)) {
		FAIL();
	}
	return true;
}


bool union_elem_code(cbor_state_t *p_state)
{
	if (!restore_backup(p_state, FLAG_RESTORE, p_state->elem_count)) {
		FAIL();
	}
	return true;
}

bool union_end_code(cbor_state_t *p_state)
{
	if (!restore_backup(p_state, FLAG_DISCARD, p_state->elem_count)) {
		FAIL();
	}
	return true;
}

bool entry_function(const uint8_t *p_payload, size_t payload_len,
		const void *p_struct, size_t *p_payload_len_out,
		cbor_encoder_t func, size_t elem_count, size_t num_backups)
{
	cbor_state_t state = {
		.p_payload = p_payload,
		.p_payload_end = p_payload + payload_len,
		.elem_count = elem_count,
	};

	cbor_state_t state_backups[num_backups + 1];

	cbor_state_backups_t backups = {
		.p_backup_list = state_backups,
		.current_backup = 0,
		.num_backups = num_backups + 1,
	};

	state.p_backups = &backups;

	bool result = func(&state, p_struct);

	if (result && (p_payload_len_out != NULL)) {
		*p_payload_len_out = MIN(payload_len,
				(size_t)state.p_payload - (size_t)p_payload);
	}
	return result;
}
