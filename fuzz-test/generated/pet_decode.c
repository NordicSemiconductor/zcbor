/*
 * Generated with cddl_gen.py (https://github.com/oyvindronningstad/cddl_gen)
 * Generated with a default_max_qty of 3
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "cbor_decode.h"
#include "pet_decode.h"

#if DEFAULT_MAX_QTY != 3
#error "The type file was generated with a different default_max_qty than this file"
#endif


static bool decode_Pet(
		cbor_state_t *state, struct Pet *result)
{
	cbor_print("%s\n", __func__);
	bool int_res;

	bool tmp_result = (((list_start_decode(state) && (int_res = (((list_start_decode(state) && (int_res = (multi_decode(1, 3, &(*result)._Pet_name_tstr_count, (void *)tstrx_decode, state, (&(*result)._Pet_name_tstr), sizeof(cbor_string_type_t))), ((list_end_decode(state)) && int_res))))
	&& ((bstrx_decode(state, (&(*result)._Pet_birthday))))
	&& ((((uintx32_decode(state, (uint32_t *)&(*result)._Pet_species_choice))) && ((((*result)._Pet_species_choice == _Pet_species_cat) && ((1)))
	|| (((*result)._Pet_species_choice == _Pet_species_dog) && ((1)))
	|| (((*result)._Pet_species_choice == _Pet_species_other) && ((1))))))), ((list_end_decode(state)) && int_res)))));

	if (!tmp_result)
		cbor_trace();

	return tmp_result;
}



__attribute__((unused)) static bool type_test_decode_Pet(
		struct Pet *result)
{
	/* This function should not be called, it is present only to test that
	 * the types of the function and struct match, since this information
	 * is lost with the casts in the entry function.
	 */
	return decode_Pet(NULL, result);
}


bool cbor_decode_Pet(
		const uint8_t *payload, uint32_t payload_len,
		struct Pet *result,
		uint32_t *payload_len_out)
{
	return entry_function(payload, payload_len, (const void *)result,
		payload_len_out, (void *)decode_Pet,
		1, 2);
}
