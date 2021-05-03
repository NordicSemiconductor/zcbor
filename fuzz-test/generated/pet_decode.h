/*
 * Generated with cddl_gen.py (https://github.com/oyvindronningstad/cddl_gen)
 * Generated with a default_max_qty of 3
 */

#ifndef PET_DECODE_H__
#define PET_DECODE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "cbor_decode.h"
#include "types_pet_decode.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif


bool cbor_decode_Pet(
		const uint8_t *payload, uint32_t payload_len,
		struct Pet *result,
		uint32_t *payload_len_out);


#endif /* PET_DECODE_H__ */
