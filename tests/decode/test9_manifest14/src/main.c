/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#ifdef MANIFEST16
	#include "manifest16_decode.h"
#else
	#include "manifest14_decode.h"
#endif
#include "zcbor_debug.h" // Enables use of print functions when debugging tests.
#include <example0.h>


ZTEST(cbor_decode_test9, test_suit14_ex0_auth)
{
	struct SUIT_Envelope envelope;
	struct SUIT_Digest *digest;
	struct COSE_Sign1 *cose_sign1;
	size_t out_len;
	uint8_t exp_digest[] = {
		0xa6, 0xc4, 0x59, 0x0a, 0xc5, 0x30, 0x43, 0xa9,
		0x8e, 0x8c, 0x41, 0x06, 0xe1, 0xe3, 0x1b, 0x30,
		0x55, 0x16, 0xd7, 0xcf, 0x0a, 0x65, 0x5e, 0xdd,
		0xfa, 0xc6, 0xd4, 0x5c, 0x81, 0x0e, 0x03, 0x6a,
	};
	uint8_t exp_signature[] = {
		0xd1, 0x1a, 0x2d, 0xd9, 0x61, 0x0f, 0xb6, 0x2a,
		0x70, 0x73, 0x35, 0xf5, 0x84, 0x07, 0x92, 0x25,
		0x70, 0x9f, 0x96, 0xe8, 0x11, 0x7e, 0x7e, 0xee,
		0xd9, 0x8a, 0x2f, 0x20, 0x7d, 0x05, 0xc8, 0xec,
		0xfb, 0xa1, 0x75, 0x52, 0x08, 0xf6, 0xab, 0xea,
		0x97, 0x7b, 0x8a, 0x6e, 0xfe, 0x3b, 0xc2, 0xca,
		0x32, 0x15, 0xe1, 0x19, 0x3b, 0xe2, 0x01, 0x46,
		0x7d, 0x05, 0x2b, 0x42, 0xdb, 0x6b, 0x72, 0x87,
	};

	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Envelope_Tagged(example0, sizeof(example0), &envelope, &out_len), NULL);
	zassert_equal(sizeof(example0), out_len, NULL);

	digest = &envelope._SUIT_Envelope_suit_authentication_wrapper_cbor._SUIT_Authentication_SUIT_Digest_bstr_cbor;
	zassert_equal(_suit_cose_hash_algs__cose_alg_sha_256,
		digest->_SUIT_Digest_suit_digest_algorithm_id._suit_cose_hash_algs_choice, NULL);
	zassert_equal(sizeof(exp_digest),
		digest->_SUIT_Digest_suit_digest_bytes.len, NULL);
	zassert_mem_equal(exp_digest,
		digest->_SUIT_Digest_suit_digest_bytes.value,
		digest->_SUIT_Digest_suit_digest_bytes.len, NULL);
	zassert_equal(1, envelope._SUIT_Envelope_suit_authentication_wrapper_cbor._SUIT_Authentication_Block_bstr_count, NULL);
	zassert_equal(_SUIT_Authentication_Block__COSE_Sign1_Tagged,
		envelope._SUIT_Envelope_suit_authentication_wrapper_cbor._SUIT_Authentication_Block_bstr[0]._SUIT_Authentication_Block_bstr_cbor._SUIT_Authentication_Block_choice, NULL);
	cose_sign1 = &envelope._SUIT_Envelope_suit_authentication_wrapper_cbor._SUIT_Authentication_Block_bstr[0]._SUIT_Authentication_Block_bstr_cbor._SUIT_Authentication_Block__COSE_Sign1_Tagged;
	zassert_equal(_empty_or_serialized_map_header_map_bstr,
		cose_sign1->_COSE_Sign1__Headers._Headers_protected._empty_or_serialized_map_choice, NULL);
	zassert_equal(0, cose_sign1->_COSE_Sign1__Headers._Headers_protected._empty_or_serialized_map_header_map_bstr_cbor._header_map_label_count, NULL);
	zassert_true(cose_sign1->_COSE_Sign1__Headers._Headers_protected._empty_or_serialized_map_header_map_bstr_cbor._header_map__Generic_Headers._Generic_Headers_uint1union_present, NULL);
	zassert_false(cose_sign1->_COSE_Sign1__Headers._Headers_protected._empty_or_serialized_map_header_map_bstr_cbor._header_map__Generic_Headers._Generic_Headers___label_present, NULL);
	zassert_false(cose_sign1->_COSE_Sign1__Headers._Headers_protected._empty_or_serialized_map_header_map_bstr_cbor._header_map__Generic_Headers._Generic_Headers_uint3union_present, NULL);
	zassert_false(cose_sign1->_COSE_Sign1__Headers._Headers_protected._empty_or_serialized_map_header_map_bstr_cbor._header_map__Generic_Headers._Generic_Headers_uint4bstr_present, NULL);
	zassert_false(cose_sign1->_COSE_Sign1__Headers._Headers_protected._empty_or_serialized_map_header_map_bstr_cbor._header_map__Generic_Headers._Generic_Headers_uint5bstr_present, NULL);
	zassert_false(cose_sign1->_COSE_Sign1__Headers._Headers_protected._empty_or_serialized_map_header_map_bstr_cbor._header_map__Generic_Headers._Generic_Headers_uint6bstr_present, NULL);
	zassert_equal(_Generic_Headers_uint1union_int,
		cose_sign1->_COSE_Sign1__Headers._Headers_protected._empty_or_serialized_map_header_map_bstr_cbor._header_map__Generic_Headers._Generic_Headers_uint1union._Generic_Headers_uint1union_choice, NULL);
	zassert_equal(-7,
		cose_sign1->_COSE_Sign1__Headers._Headers_protected._empty_or_serialized_map_header_map_bstr_cbor._header_map__Generic_Headers._Generic_Headers_uint1union._Generic_Headers_uint1union_int, NULL);
	zassert_equal(sizeof(exp_signature), cose_sign1->_COSE_Sign1_signature.len, NULL);
	zassert_mem_equal(exp_signature, cose_sign1->_COSE_Sign1_signature.value,
		cose_sign1->_COSE_Sign1_signature.len, NULL);
	zassert_equal(_COSE_Sign1_payload_nil, cose_sign1->_COSE_Sign1_payload_choice, NULL);
}


/* Check the common sequence. */
ZTEST(cbor_decode_test9, test_suit14_ex0_common_sequence)
{
	struct SUIT_Envelope envelope;
	struct SUIT_Manifest manifest;
	struct SUIT_Common_Sequence common_sequence;
	struct SUIT_Command_Sequence command_sequence;
	struct SUIT_Parameters_ *parameter;
	struct SUIT_Condition_ *condition;
	size_t out_len;
	uint8_t exp_vendor_id[] = {
		0xfa, 0x6b, 0x4a, 0x53, 0xd5, 0xad, 0x5f, 0xdf,
		0xbe, 0x9d, 0xe6, 0x63, 0xe4, 0xd4, 0x1f, 0xfe,
	};
	uint8_t exp_class_id[] = {
		0x14, 0x92, 0xaf, 0x14, 0x25, 0x69, 0x5e, 0x48,
		0xbf, 0x42, 0x9b, 0x2d, 0x51, 0xf2, 0xab, 0x45,
	};
	uint8_t exp_digest[] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
		0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
		0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
	};
	uint32_t exp_rep_policy = (
		(1 << _suit_reporting_bits_suit_send_record_success)
		| (1 << _suit_reporting_bits_suit_send_record_failure)
		| (1 << _suit_reporting_bits_suit_send_sysinfo_success)
		| (1 << _suit_reporting_bits_suit_send_sysinfo_failure));

	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Envelope_Tagged(example0, sizeof(example0), &envelope, &out_len), NULL);
	zassert_equal(sizeof(example0), out_len, NULL);
	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Manifest(envelope._SUIT_Envelope_suit_manifest.value,
		envelope._SUIT_Envelope_suit_manifest.len, &manifest, &out_len), NULL);
	zassert_equal(envelope._SUIT_Envelope_suit_manifest.len, out_len, NULL);
	zassert_true(manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_components_present, NULL);
	zassert_true(manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence_present, NULL);
	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Common_Sequence(
		manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence._SUIT_Common_suit_common_sequence.value,
		manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence._SUIT_Common_suit_common_sequence.len,
		&common_sequence, &out_len), NULL);
	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Command_Sequence(
		manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence._SUIT_Common_suit_common_sequence.value,
		manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence._SUIT_Common_suit_common_sequence.len,
		&command_sequence, &out_len), NULL);
	zassert_equal(
		manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence._SUIT_Common_suit_common_sequence.len,
		out_len, NULL);
	zassert_equal(3, common_sequence._SUIT_Common_Sequence_union_count, NULL);
	zassert_equal(_SUIT_Common_Sequence_union__SUIT_Common_Commands, common_sequence._SUIT_Common_Sequence_union[0]._SUIT_Common_Sequence_union_choice, NULL);
	zassert_equal(_SUIT_Common_Commands___suit_directive_override_parameters, common_sequence._SUIT_Common_Sequence_union[0]._SUIT_Common_Sequence_union__SUIT_Common_Commands._SUIT_Common_Commands_choice, NULL);
	zassert_equal(4, common_sequence._SUIT_Common_Sequence_union[0]._SUIT_Common_Sequence_union__SUIT_Common_Commands.___suit_directive_override_parameters_map__SUIT_Parameters_count, NULL);
	parameter = &common_sequence._SUIT_Common_Sequence_union[0]._SUIT_Common_Sequence_union__SUIT_Common_Commands.___suit_directive_override_parameters_map__SUIT_Parameters[0].___suit_directive_override_parameters_map__SUIT_Parameters;
	zassert_equal(
		_SUIT_Parameters_suit_parameter_vendor_identifier,
		parameter->_SUIT_Parameters_choice, NULL);
	zassert_equal(
		_SUIT_Parameters_suit_parameter_vendor_identifier__RFC4122_UUID,
		parameter->_SUIT_Parameters_suit_parameter_vendor_identifier_choice, NULL);
	zassert_equal(
		parameter->_SUIT_Parameters_suit_parameter_vendor_identifier__RFC4122_UUID.len,
		sizeof(exp_vendor_id), NULL);
	zassert_mem_equal(exp_vendor_id,
		parameter->_SUIT_Parameters_suit_parameter_vendor_identifier__RFC4122_UUID.value,
		parameter->_SUIT_Parameters_suit_parameter_vendor_identifier__RFC4122_UUID.len, NULL);

	parameter = &common_sequence._SUIT_Common_Sequence_union[0]._SUIT_Common_Sequence_union__SUIT_Common_Commands.___suit_directive_override_parameters_map__SUIT_Parameters[1].___suit_directive_override_parameters_map__SUIT_Parameters;
	zassert_equal(
		_SUIT_Parameters_suit_parameter_class_identifier,
		parameter->_SUIT_Parameters_choice, NULL);
	zassert_equal(
		parameter->_SUIT_Parameters_suit_parameter_class_identifier.len,
		sizeof(exp_class_id), NULL);
	zassert_mem_equal(exp_class_id,
		parameter->_SUIT_Parameters_suit_parameter_class_identifier.value,
		parameter->_SUIT_Parameters_suit_parameter_class_identifier.len, NULL);

	parameter = &common_sequence._SUIT_Common_Sequence_union[0]._SUIT_Common_Sequence_union__SUIT_Common_Commands.___suit_directive_override_parameters_map__SUIT_Parameters[2].___suit_directive_override_parameters_map__SUIT_Parameters;
	zassert_equal(
		_SUIT_Parameters_suit_parameter_image_digest,
		parameter->_SUIT_Parameters_choice, NULL);
	zassert_equal(
		parameter->_SUIT_Parameters_suit_parameter_image_digest_cbor._SUIT_Digest_suit_digest_bytes.len,
		sizeof(exp_digest), NULL);
	zassert_equal(_suit_cose_hash_algs__cose_alg_sha_256,
		parameter->_SUIT_Parameters_suit_parameter_image_digest_cbor._SUIT_Digest_suit_digest_algorithm_id._suit_cose_hash_algs_choice, NULL);
	zassert_mem_equal(exp_digest,
		parameter->_SUIT_Parameters_suit_parameter_image_digest_cbor._SUIT_Digest_suit_digest_bytes.value,
		parameter->_SUIT_Parameters_suit_parameter_image_digest_cbor._SUIT_Digest_suit_digest_bytes.len, NULL);

	parameter = &common_sequence._SUIT_Common_Sequence_union[0]._SUIT_Common_Sequence_union__SUIT_Common_Commands.___suit_directive_override_parameters_map__SUIT_Parameters[3].___suit_directive_override_parameters_map__SUIT_Parameters;
	zassert_equal(
		_SUIT_Parameters_suit_parameter_image_size,
		parameter->_SUIT_Parameters_choice, NULL);
	zassert_equal(34768, parameter->_SUIT_Parameters_suit_parameter_image_size, NULL);

	zassert_equal(_SUIT_Common_Sequence_union__SUIT_Condition, common_sequence._SUIT_Common_Sequence_union[1]._SUIT_Common_Sequence_union_choice, NULL);
	condition = &common_sequence._SUIT_Common_Sequence_union[1]._SUIT_Common_Sequence_union__SUIT_Condition;
	zassert_equal(_SUIT_Condition___suit_condition_vendor_identifier,
		condition->_SUIT_Condition_choice, NULL);
	zassert_equal(exp_rep_policy,
		condition->_SUIT_Condition___suit_condition_vendor_identifier__SUIT_Rep_Policy, NULL);
	zassert_equal(exp_rep_policy,
		condition->_SUIT_Condition___suit_condition_class_identifier__SUIT_Rep_Policy, NULL);

	zassert_equal(_SUIT_Common_Sequence_union__SUIT_Condition, common_sequence._SUIT_Common_Sequence_union[2]._SUIT_Common_Sequence_union_choice, NULL);
	condition = &common_sequence._SUIT_Common_Sequence_union[2]._SUIT_Common_Sequence_union__SUIT_Condition;
	zassert_equal(_SUIT_Condition___suit_condition_class_identifier,
		condition->_SUIT_Condition_choice, NULL);
	zassert_equal(exp_rep_policy,
		condition->_SUIT_Condition___suit_condition_class_identifier__SUIT_Rep_Policy, NULL);
}

/* Check the common sequence when decoded as a command sequence.
 * This should work fine because SUIT_Common_Sequence is a subset of
 * SUIT_Command_Sequence.
 */
ZTEST(cbor_decode_test9, test_suit14_ex0_common_sequence_as_command_sequence)
{
	struct SUIT_Envelope envelope;
	struct SUIT_Manifest manifest;
	struct SUIT_Common_Sequence common_sequence;
	struct SUIT_Command_Sequence command_sequence;
	struct SUIT_Parameters_ *parameter;
	struct SUIT_Condition_ *condition;
	size_t out_len;
	uint8_t exp_vendor_id[] = {
		0xfa, 0x6b, 0x4a, 0x53, 0xd5, 0xad, 0x5f, 0xdf,
		0xbe, 0x9d, 0xe6, 0x63, 0xe4, 0xd4, 0x1f, 0xfe,
	};
	uint8_t exp_class_id[] = {
		0x14, 0x92, 0xaf, 0x14, 0x25, 0x69, 0x5e, 0x48,
		0xbf, 0x42, 0x9b, 0x2d, 0x51, 0xf2, 0xab, 0x45,
	};
	uint8_t exp_digest[] = {
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
		0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
		0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
	};
	uint32_t exp_rep_policy = (
		(1 << _suit_reporting_bits_suit_send_record_success)
		| (1 << _suit_reporting_bits_suit_send_record_failure)
		| (1 << _suit_reporting_bits_suit_send_sysinfo_success)
		| (1 << _suit_reporting_bits_suit_send_sysinfo_failure));

	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Envelope_Tagged(example0, sizeof(example0), &envelope, &out_len), NULL);
	zassert_equal(sizeof(example0), out_len, NULL);
	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Manifest(envelope._SUIT_Envelope_suit_manifest.value,
		envelope._SUIT_Envelope_suit_manifest.len, &manifest, &out_len), NULL);
	zassert_equal(envelope._SUIT_Envelope_suit_manifest.len, out_len, NULL);
	zassert_true(manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_components_present, NULL);
	zassert_true(manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence_present, NULL);
	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Common_Sequence(
		manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence._SUIT_Common_suit_common_sequence.value,
		manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence._SUIT_Common_suit_common_sequence.len,
		&common_sequence, &out_len), NULL);
	zassert_equal(
		manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence._SUIT_Common_suit_common_sequence.len,
		out_len, NULL);
	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Command_Sequence(
		manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence._SUIT_Common_suit_common_sequence.value,
		manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence._SUIT_Common_suit_common_sequence.len,
		&command_sequence, &out_len), NULL);
	zassert_equal(
		manifest._SUIT_Manifest_suit_common_cbor._SUIT_Common_suit_common_sequence._SUIT_Common_suit_common_sequence.len,
		out_len, NULL);
	zassert_equal(3, command_sequence._SUIT_Command_Sequence_union_count, NULL);
	zassert_equal(_SUIT_Command_Sequence_union__SUIT_Directive, command_sequence._SUIT_Command_Sequence_union[0]._SUIT_Command_Sequence_union_choice, NULL);
	zassert_equal(_SUIT_Directive___suit_directive_override_parameters, command_sequence._SUIT_Command_Sequence_union[0]._SUIT_Command_Sequence_union__SUIT_Directive._SUIT_Directive_choice, NULL);
	zassert_equal(4, command_sequence._SUIT_Command_Sequence_union[0]._SUIT_Command_Sequence_union__SUIT_Directive.___suit_directive_override_parameters_map__SUIT_Parameters_count, NULL);

	parameter = &command_sequence._SUIT_Command_Sequence_union[0]._SUIT_Command_Sequence_union__SUIT_Directive.___suit_directive_override_parameters_map__SUIT_Parameters[0].___suit_directive_override_parameters_map__SUIT_Parameters;
	zassert_equal(
		_SUIT_Parameters_suit_parameter_vendor_identifier,
		parameter->_SUIT_Parameters_choice, NULL);
	zassert_equal(
		_SUIT_Parameters_suit_parameter_vendor_identifier__RFC4122_UUID,
		parameter->_SUIT_Parameters_suit_parameter_vendor_identifier_choice, NULL);
	zassert_equal(
		parameter->_SUIT_Parameters_suit_parameter_vendor_identifier__RFC4122_UUID.len,
		sizeof(exp_vendor_id), NULL);
	zassert_mem_equal(exp_vendor_id,
		parameter->_SUIT_Parameters_suit_parameter_vendor_identifier__RFC4122_UUID.value,
		parameter->_SUIT_Parameters_suit_parameter_vendor_identifier__RFC4122_UUID.len, NULL);

	parameter = &command_sequence._SUIT_Command_Sequence_union[0]._SUIT_Command_Sequence_union__SUIT_Directive.___suit_directive_override_parameters_map__SUIT_Parameters[1].___suit_directive_override_parameters_map__SUIT_Parameters;
	zassert_equal(
		_SUIT_Parameters_suit_parameter_class_identifier,
		parameter->_SUIT_Parameters_choice, NULL);
	zassert_equal(
		parameter->_SUIT_Parameters_suit_parameter_class_identifier.len,
		sizeof(exp_class_id), NULL);
	zassert_mem_equal(exp_class_id,
		parameter->_SUIT_Parameters_suit_parameter_class_identifier.value,
		parameter->_SUIT_Parameters_suit_parameter_class_identifier.len, NULL);

	parameter = &command_sequence._SUIT_Command_Sequence_union[0]._SUIT_Command_Sequence_union__SUIT_Directive.___suit_directive_override_parameters_map__SUIT_Parameters[2].___suit_directive_override_parameters_map__SUIT_Parameters;
	zassert_equal(
		_SUIT_Parameters_suit_parameter_image_digest,
		parameter->_SUIT_Parameters_choice, NULL);
	zassert_equal(
		parameter->_SUIT_Parameters_suit_parameter_image_digest_cbor._SUIT_Digest_suit_digest_bytes.len,
		sizeof(exp_digest), NULL);
	zassert_equal(_suit_cose_hash_algs__cose_alg_sha_256,
		parameter->_SUIT_Parameters_suit_parameter_image_digest_cbor._SUIT_Digest_suit_digest_algorithm_id._suit_cose_hash_algs_choice, NULL);
	zassert_mem_equal(exp_digest,
		parameter->_SUIT_Parameters_suit_parameter_image_digest_cbor._SUIT_Digest_suit_digest_bytes.value,
		parameter->_SUIT_Parameters_suit_parameter_image_digest_cbor._SUIT_Digest_suit_digest_bytes.len, NULL);

	parameter = &command_sequence._SUIT_Command_Sequence_union[0]._SUIT_Command_Sequence_union__SUIT_Directive.___suit_directive_override_parameters_map__SUIT_Parameters[3].___suit_directive_override_parameters_map__SUIT_Parameters;
	zassert_equal(
		_SUIT_Parameters_suit_parameter_image_size,
		parameter->_SUIT_Parameters_choice, NULL);
	zassert_equal(34768, parameter->_SUIT_Parameters_suit_parameter_image_size, NULL);

	zassert_equal(_SUIT_Command_Sequence_union__SUIT_Condition, command_sequence._SUIT_Command_Sequence_union[1]._SUIT_Command_Sequence_union_choice, NULL);
	condition = &command_sequence._SUIT_Command_Sequence_union[1]._SUIT_Command_Sequence_union__SUIT_Condition;
	zassert_equal(_SUIT_Condition___suit_condition_vendor_identifier,
		condition->_SUIT_Condition_choice, NULL);
	zassert_equal(exp_rep_policy,
		condition->_SUIT_Condition___suit_condition_vendor_identifier__SUIT_Rep_Policy, NULL);
	zassert_equal(exp_rep_policy,
		condition->_SUIT_Condition___suit_condition_class_identifier__SUIT_Rep_Policy, NULL);

	zassert_equal(_SUIT_Command_Sequence_union__SUIT_Condition, command_sequence._SUIT_Command_Sequence_union[2]._SUIT_Command_Sequence_union_choice, NULL);
	condition = &command_sequence._SUIT_Command_Sequence_union[2]._SUIT_Command_Sequence_union__SUIT_Condition;
	zassert_equal(_SUIT_Condition___suit_condition_class_identifier,
		condition->_SUIT_Condition_choice, NULL);
	zassert_equal(exp_rep_policy,
		condition->_SUIT_Condition___suit_condition_class_identifier__SUIT_Rep_Policy, NULL);
}

ZTEST(cbor_decode_test9, test_suit14_ex0_validate_run)
{
	struct SUIT_Envelope envelope;
	struct SUIT_Manifest manifest;
	struct SUIT_Command_Sequence command_sequence;
	struct SUIT_Condition_ *condition;
	struct SUIT_Directive_ *directive;
	size_t out_len;
	uint32_t exp_rep_policy1 = (
		(1 << _suit_reporting_bits_suit_send_record_success)
		| (1 << _suit_reporting_bits_suit_send_record_failure)
		| (1 << _suit_reporting_bits_suit_send_sysinfo_success)
		| (1 << _suit_reporting_bits_suit_send_sysinfo_failure));
	uint32_t exp_rep_policy2 = (1 << _suit_reporting_bits_suit_send_record_failure);

	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Envelope_Tagged(example0, sizeof(example0), &envelope, &out_len), NULL);
	zassert_equal(sizeof(example0), out_len, NULL);
	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Manifest(envelope._SUIT_Envelope_suit_manifest.value,
		envelope._SUIT_Envelope_suit_manifest.len, &manifest, &out_len), NULL);
	zassert_equal(envelope._SUIT_Envelope_suit_manifest.len, out_len, NULL);
	zassert_true(manifest._SUIT_Manifest__SUIT_Unseverable_Members._SUIT_Unseverable_Members_suit_validate_present, NULL);
	zassert_false(manifest._SUIT_Manifest__SUIT_Unseverable_Members._SUIT_Unseverable_Members_suit_load_present, NULL);
	zassert_true(manifest._SUIT_Manifest__SUIT_Unseverable_Members._SUIT_Unseverable_Members_suit_run_present, NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Command_Sequence(
		manifest._SUIT_Manifest__SUIT_Unseverable_Members._SUIT_Unseverable_Members_suit_validate._SUIT_Unseverable_Members_suit_validate.value,
		manifest._SUIT_Manifest__SUIT_Unseverable_Members._SUIT_Unseverable_Members_suit_validate._SUIT_Unseverable_Members_suit_validate.len,
		&command_sequence, &out_len), NULL);
	zassert_equal(
		manifest._SUIT_Manifest__SUIT_Unseverable_Members._SUIT_Unseverable_Members_suit_validate._SUIT_Unseverable_Members_suit_validate.len,
		out_len, NULL);
	zassert_equal(1, command_sequence._SUIT_Command_Sequence_union_count, NULL);
	zassert_equal(_SUIT_Command_Sequence_union__SUIT_Condition, command_sequence._SUIT_Command_Sequence_union[0]._SUIT_Command_Sequence_union_choice, NULL);
	condition = &command_sequence._SUIT_Command_Sequence_union[0]._SUIT_Command_Sequence_union__SUIT_Condition;
	zassert_equal(_SUIT_Condition___suit_condition_image_match,
		condition->_SUIT_Condition_choice, NULL);
	zassert_equal(exp_rep_policy1,
		condition->_SUIT_Condition___suit_condition_image_match__SUIT_Rep_Policy, NULL);

	zassert_equal(ZCBOR_SUCCESS, cbor_decode_SUIT_Command_Sequence(
		manifest._SUIT_Manifest__SUIT_Unseverable_Members._SUIT_Unseverable_Members_suit_run._SUIT_Unseverable_Members_suit_run.value,
		manifest._SUIT_Manifest__SUIT_Unseverable_Members._SUIT_Unseverable_Members_suit_run._SUIT_Unseverable_Members_suit_run.len,
		&command_sequence, &out_len), NULL);
	zassert_equal(
		manifest._SUIT_Manifest__SUIT_Unseverable_Members._SUIT_Unseverable_Members_suit_run._SUIT_Unseverable_Members_suit_run.len,
		out_len, NULL);
	zassert_equal(1, command_sequence._SUIT_Command_Sequence_union_count, NULL);
	zassert_equal(_SUIT_Command_Sequence_union__SUIT_Directive, command_sequence._SUIT_Command_Sequence_union[0]._SUIT_Command_Sequence_union_choice, NULL);
	directive = &command_sequence._SUIT_Command_Sequence_union[0]._SUIT_Command_Sequence_union__SUIT_Directive;
	zassert_equal(_SUIT_Directive___suit_directive_run,
		directive->_SUIT_Directive_choice, NULL);
	zassert_equal(exp_rep_policy2,
		directive->_SUIT_Directive___suit_directive_run__SUIT_Rep_Policy, NULL);

}

ZTEST_SUITE(cbor_decode_test9, NULL, NULL, NULL, NULL, NULL);
