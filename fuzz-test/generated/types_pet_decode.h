/*
 * Generated with cddl_gen.py (https://github.com/oyvindronningstad/cddl_gen)
 * Generated with a default_max_qty of 3
 */

#ifndef TYPES_PET_DECODE_H__
#define TYPES_PET_DECODE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "cbor_decode.h"

#define DEFAULT_MAX_QTY 3

struct Pet {
 	cbor_string_type_t _Pet_name_tstr[3];
	uint32_t _Pet_name_tstr_count;
	cbor_string_type_t _Pet_birthday;
	union {
		uint32_t _Pet_species_cat;
		uint32_t _Pet_species_dog;
		uint32_t _Pet_species_other;
	};
	enum {
		_Pet_species_cat = 1,
		_Pet_species_dog = 2,
		_Pet_species_other = 3,
	} _Pet_species_choice;
};


#endif /* TYPES_PET_DECODE_H__ */
