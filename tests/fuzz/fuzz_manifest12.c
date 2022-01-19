#include "manifest12_decode.h"
#include "main_entry.h"

bool fuzz_one_input(const uint8_t *data, size_t size)
{
    size_t payload_len_out = 0;
    struct SUIT_Envelope result;
    struct SUIT_Envelope result2;
    struct SUIT_Command_Sequence command_seq;

    bool ret = cbor_decode_SUIT_Envelope_Tagged(data, size,
                               &result,
                               &payload_len_out);
    if (!ret) {
        return ret;
    }

    for (int i = 0; i < result._SUIT_Envelope_suit_integrated_dependency_key_count; i++) {
        ret = cbor_decode_SUIT_Envelope(
            result._SUIT_Envelope_suit_integrated_dependency_key[i]._SUIT_Envelope_suit_integrated_dependency_key.value,
            result._SUIT_Envelope_suit_integrated_dependency_key[i]._SUIT_Envelope_suit_integrated_dependency_key.len,
            &result2, &payload_len_out);
        if (!ret) {
            return ret;
        }
    }

    if (result._SUIT_Envelope_suit_manifest_cbor
              ._SUIT_Manifest__SUIT_Severable_Manifest_Members
              ._SUIT_Severable_Manifest_Members_suit_dependency_resolution_present) {
        ret = cbor_decode_SUIT_Command_Sequence(
            result._SUIT_Envelope_suit_manifest_cbor
                ._SUIT_Manifest__SUIT_Severable_Manifest_Members
                ._SUIT_Severable_Manifest_Members_suit_dependency_resolution
                ._SUIT_Severable_Manifest_Members_suit_dependency_resolution.value,
            result._SUIT_Envelope_suit_manifest_cbor
                ._SUIT_Manifest__SUIT_Severable_Manifest_Members
                ._SUIT_Severable_Manifest_Members_suit_dependency_resolution
                ._SUIT_Severable_Manifest_Members_suit_dependency_resolution.len,
            &command_seq, &payload_len_out);
        if (!ret) {
            return ret;
        }
    }

    if (result._SUIT_Envelope_suit_manifest_cbor
              ._SUIT_Manifest__SUIT_Severable_Manifest_Members
              ._SUIT_Severable_Manifest_Members_suit_payload_fetch_present) {
        ret = cbor_decode_SUIT_Command_Sequence(
            result._SUIT_Envelope_suit_manifest_cbor
                ._SUIT_Manifest__SUIT_Severable_Manifest_Members
                ._SUIT_Severable_Manifest_Members_suit_payload_fetch
                ._SUIT_Severable_Manifest_Members_suit_payload_fetch.value,
            result._SUIT_Envelope_suit_manifest_cbor
                ._SUIT_Manifest__SUIT_Severable_Manifest_Members
                ._SUIT_Severable_Manifest_Members_suit_payload_fetch
                ._SUIT_Severable_Manifest_Members_suit_payload_fetch.len,
            &command_seq, &payload_len_out);
        if (!ret) {
            return ret;
        }
    }

    if (result._SUIT_Envelope_suit_manifest_cbor
              ._SUIT_Manifest__SUIT_Severable_Manifest_Members
              ._SUIT_Severable_Manifest_Members_suit_install_present) {
        ret = cbor_decode_SUIT_Command_Sequence(
            result._SUIT_Envelope_suit_manifest_cbor
                ._SUIT_Manifest__SUIT_Severable_Manifest_Members
                ._SUIT_Severable_Manifest_Members_suit_install
                ._SUIT_Severable_Manifest_Members_suit_install.value,
            result._SUIT_Envelope_suit_manifest_cbor
                ._SUIT_Manifest__SUIT_Severable_Manifest_Members
                ._SUIT_Severable_Manifest_Members_suit_install
                ._SUIT_Severable_Manifest_Members_suit_install.len,
            &command_seq, &payload_len_out);
        if (!ret) {
            return ret;
        }
    }

    if (result._SUIT_Envelope_suit_manifest_cbor
              ._SUIT_Manifest__SUIT_Unseverable_Members
              ._SUIT_Unseverable_Members_suit_validate_present) {
        ret = cbor_decode_SUIT_Command_Sequence(
            result._SUIT_Envelope_suit_manifest_cbor
                ._SUIT_Manifest__SUIT_Unseverable_Members
                ._SUIT_Unseverable_Members_suit_validate
                ._SUIT_Unseverable_Members_suit_validate.value,
            result._SUIT_Envelope_suit_manifest_cbor
                ._SUIT_Manifest__SUIT_Unseverable_Members
                ._SUIT_Unseverable_Members_suit_validate
                ._SUIT_Unseverable_Members_suit_validate.len,
            &command_seq, &payload_len_out);
        if (!ret) {
            return ret;
        }
    }

    if (result._SUIT_Envelope_suit_manifest_cbor
              ._SUIT_Manifest__SUIT_Unseverable_Members
              ._SUIT_Unseverable_Members_suit_load_present) {
        ret = cbor_decode_SUIT_Command_Sequence(
            result._SUIT_Envelope_suit_manifest_cbor
                ._SUIT_Manifest__SUIT_Unseverable_Members
                ._SUIT_Unseverable_Members_suit_load
                ._SUIT_Unseverable_Members_suit_load.value,
            result._SUIT_Envelope_suit_manifest_cbor
                ._SUIT_Manifest__SUIT_Unseverable_Members
                ._SUIT_Unseverable_Members_suit_load
                ._SUIT_Unseverable_Members_suit_load.len,
            &command_seq, &payload_len_out);
        if (!ret) {
            return ret;
        }
    }

    if (result._SUIT_Envelope_suit_manifest_cbor
              ._SUIT_Manifest__SUIT_Unseverable_Members
              ._SUIT_Unseverable_Members_suit_run_present) {
        ret = cbor_decode_SUIT_Command_Sequence(
            result._SUIT_Envelope_suit_manifest_cbor
                ._SUIT_Manifest__SUIT_Unseverable_Members
                ._SUIT_Unseverable_Members_suit_run
                ._SUIT_Unseverable_Members_suit_run.value,
            result._SUIT_Envelope_suit_manifest_cbor
                ._SUIT_Manifest__SUIT_Unseverable_Members
                ._SUIT_Unseverable_Members_suit_run
                ._SUIT_Unseverable_Members_suit_run.len,
            &command_seq, &payload_len_out);
        if (!ret) {
            return ret;
        }
    }
    return ret;
}
