/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CBOR_COMMON_H__
#define CBOR_COMMON_H__
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


/** Convenience type that allows pointing to strings directly inside the payload
 *  without the need to copy out.
 */
typedef struct
{
	const uint8_t *value;
	size_t len;
} zcbor_string_type_t;

#ifdef ZCBOR_VERBOSE
#include <sys/printk.h>
#define zcbor_trace() (printk("bytes left: %zu, byte: 0x%x, elem_count: 0x%x, %s:%d\n",\
	(size_t)state->payload_end - (size_t)state->payload, *state->payload, state->elem_count,\
	__FILE__, __LINE__))
#define zcbor_assert(expr, ...) \
do { \
	if (!(expr)) { \
		printk("ASSERTION \n  \"" #expr \
			"\"\nfailed at %s:%d with message:\n  ", \
			__FILE__, __LINE__); \
		printk(__VA_ARGS__);\
		return false; \
	} \
} while(0)
#define zcbor_print(...) printk(__VA_ARGS__)
#else
#define zcbor_trace() ((void)state)
#define zcbor_assert(...)
#define zcbor_print(...)
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif


struct zcbor_state_backups_s;

typedef struct zcbor_state_backups_s zcbor_state_backups_t;

typedef struct{
union {
	uint8_t *payload_mut;
	uint8_t const *payload; /**< The current place in the payload. Will be
	                             updated when an element is correctly
	                             processed. */
};
	uint8_t const *payload_bak; /**< Temporary backup of payload. */
	uint32_t elem_count; /**< The current element is part of a LIST or a MAP,
	                          and this keeps count of how many elements are
	                          expected. This will be checked before processing
	                          and decremented if the element is correctly
	                          processed. */
	uint8_t const *payload_end; /**< The end of the payload. This will be
	                                 checked against payload before
	                                 processing each element. */
	zcbor_state_backups_t *backups;
} zcbor_state_t;

struct zcbor_state_backups_s{
	zcbor_state_t *backup_list;
	uint32_t current_backup;
	uint32_t num_backups;
};

/** Function pointer type used with zcbor_multi_decode.
 *
 * This type is compatible with all decoding functions here and in the generated
 * code, except for zcbor_multi_decode.
 */
typedef bool(zcbor_encoder_t)(zcbor_state_t *, const void *);
typedef bool(zcbor_decoder_t)(zcbor_state_t *, void *);

/** Enumeration representing the major types available in CBOR.
 *
 * The major type is represented in the 3 first bits of the header byte.
 */
typedef enum
{
	ZCBOR_MAJOR_TYPE_PINT = 0, ///! Positive Integer
	ZCBOR_MAJOR_TYPE_NINT = 1, ///! Negative Integer
	ZCBOR_MAJOR_TYPE_BSTR = 2, ///! Byte String
	ZCBOR_MAJOR_TYPE_TSTR = 3, ///! Text String
	ZCBOR_MAJOR_TYPE_LIST = 4, ///! List
	ZCBOR_MAJOR_TYPE_MAP  = 5, ///! Map
	ZCBOR_MAJOR_TYPE_TAG  = 6, ///! Semantic Tag
	ZCBOR_MAJOR_TYPE_PRIM = 7, ///! Primitive Type
} zcbor_major_type_t;


#define ZCBOR_FAIL() \
do {\
	zcbor_trace(); \
	return false; \
} while(0)


#define ZCBOR_VALUE_IN_HEADER 23 ///! Values below this are encoded directly in the header.

#define ZCBOR_BOOL_TO_PRIM 20 ///! In CBOR, false/true have the values 20/21

#define ZCBOR_FLAG_RESTORE 1UL ///! Restore from the backup.
#define ZCBOR_FLAG_CONSUME 2UL ///! Consume the backup.
#define ZCBOR_FLAG_TRANSFER_PAYLOAD 4UL ///! Keep the pre-restore payload after restoring.

/** Take a backup of the current state. Overwrite the current elem_count. */
bool zcbor_new_backup(zcbor_state_t *state, uint32_t new_elem_count);

/** Consult the most recent backup. In doing so, check whether elem_count is
 *  within max_elem_count, and return the result.
 *  Also, take action based on the flags (See ZCBOR_FLAG_*).
 */
bool zcbor_process_backup(zcbor_state_t *state, uint32_t flags, uint32_t max_elem_count);

/** Convenience function for starting encoding/decoding of a union.
 *  Takes a new backup.
 */
bool zcbor_union_start_code(zcbor_state_t *state);

/** Convenience function before encoding/decoding one element of a union.
 *  Restores the backup, without consuming it.
 */
bool zcbor_union_elem_code(zcbor_state_t *state);

/** Convenience function before encoding/decoding one element of a union.
 *  Consumes the backup without restoring it.
 */
bool zcbor_union_end_code(zcbor_state_t *state);

/** Initialize a state with backups.
 *  One of the states in the array is used as a zcbor_state_backups_t object.
 *  This means that you get a state with (n_states - 2) backups.
 *  It also means that (n_states = 2) is an invalid input, which is handled as
 *  if (n_states = 1).
 *  payload, payload_len, and elem_count are used to initialize the first state.
 *  in the array, which is the state that can be passed to cbor functions.
 */
void zcbor_new_state(zcbor_state_t *state_array, uint32_t n_states,
		const uint8_t *payload, size_t payload_len, uint32_t elem_count);

#endif /* CBOR_COMMON_H__ */
