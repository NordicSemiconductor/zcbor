/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Generated using zcbor version 0.9.99
 * https://github.com/NordicSemiconductor/zcbor
 * Generated with a --default-max-qty of 3
 */

#ifndef PET_ENCODE_H__
#define PET_ENCODE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "pet_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif

#define PET_NAME_NAMES_MAX_REP (3)
#define PET_NAME_NAMES_MIN_REP (1)
#define PET_SPECIES_CAT_VAL (1)
#define PET_SPECIES_DOG_VAL (2)
#define PET_SPECIES_OTHER_VAL (3)
#define TIMESTAMP_SIZE (8)



int cbor_encode_Pet(
		uint8_t *payload, size_t payload_len,
		const struct Pet *input,
		size_t *payload_len_out);


#ifdef __cplusplus
}
#endif

#endif /* PET_ENCODE_H__ */
