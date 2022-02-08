#!/usr/bin/env python3
#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

from unittest import TestCase, main
from subprocess import Popen, PIPE
from re import sub
from pathlib import Path
from pprint import pprint
# from ecdsa import VerifyingKey
from hashlib import sha256
import cbor2


try:
    import zcbor
except ImportError:
    print("""
The zcbor package must be installed to run these tests.
During development, install with `python3 setup.py develop` to install in a way
that picks up changes in the files without having to reinstall.
""")
    import sys
    sys.exit(1)


p_root = Path(__file__).absolute().parents[2]
p_tests = Path(p_root, 'tests')
p_manifest12 = Path(p_tests, 'cases', 'manifest12.cddl')
p_manifest14 = Path(p_tests, 'cases', 'manifest14.cddl')
p_manifest16 = Path(p_tests, 'cases', 'manifest16.cddl')
p_test_vectors12 = tuple(Path(p_tests, 'cases', f'manifest12_example{i}.cborhex') for i in range(6))
p_test_vectors14 = tuple(Path(p_tests, 'cases', f'manifest14_example{i}.cborhex') for i in range(6))
p_optional = Path(p_tests, 'cases', 'optional.cddl')
p_cose = Path(p_tests, 'cases', 'cose.cddl')
p_manifest14_priv = Path(p_tests, 'cases', 'manifest14.priv')
p_manifest14_pub = Path(p_tests, 'cases', 'manifest14.pub')
p_map_bstr_cddl = Path(p_tests, 'cases', 'map_bstr.cddl')
p_map_bstr_yaml = Path(p_tests, 'cases', 'map_bstr.yaml')


class Testn(TestCase):
    def decode_file(self, data_path, *cddl_paths):
        data = bytes.fromhex(data_path.read_text().replace("\n", ""))
        self.decode_string(data, *cddl_paths)

    def decode_string(self, data_string, *cddl_paths):
        cddl_str = " ".join((Path(p).read_text() for p in cddl_paths))
        self.my_types = zcbor.DataTranslator.from_cddl(cddl_str, 16).my_types
        cddl = self.my_types["SUIT_Envelope_Tagged"]
        self.decoded = cddl.decode_str(data_string)


class Test0(Testn):
    def __init__(self, *args, **kwargs):
        super(Test0, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors12[0], p_manifest12)

    def test_manifest_digest(self):
        self.assertEqual(
            bytes.fromhex("5c097ef64bf3bb9b494e71e1f2418eef8d466cc902f639a855ec9af3e9eddb99"),
            self.decoded.suit_authentication_wrapper.SUIT_Digest_bstr.suit_digest_bytes)

    def test_signature(self):
        self.assertEqual(
            1,
            self.decoded.suit_authentication_wrapper.SUIT_Authentication_Block_bstr[0].COSE_Sign1_Tagged.protected.uintint[0].uintint_key)
        self.assertEqual(
            -7,
            self.decoded.suit_authentication_wrapper.SUIT_Authentication_Block_bstr[0].COSE_Sign1_Tagged.protected.uintint[0].uintint)
        self.assertEqual(
            bytes.fromhex("a19fd1f23b17beed321cece7423dfb48c457b8f1f6ac83577a3c10c6773f6f3a7902376b59540920b6c5f57bac5fc8543d8f5d3d974faa2e6d03daa534b443a7"),
            self.decoded.suit_authentication_wrapper.SUIT_Authentication_Block_bstr[0].COSE_Sign1_Tagged.signature)

    def test_validate_run(self):
        self.assertEqual(
            "suit_condition_image_match",
            self.decoded.suit_manifest.SUIT_Unseverable_Members.suit_validate[0].suit_validate.union[0].SUIT_Condition.union_choice)
        self.assertEqual(
            "suit_directive_run",
            self.decoded.suit_manifest.SUIT_Unseverable_Members.suit_run[0].suit_run.union[0].SUIT_Directive.union_choice)

    def test_image_size(self):
        self.assertEqual(34768, self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[0].SUIT_Common_Commands.suit_directive_override_parameters.map[3].suit_parameter_image_size)


class Test0_Inv(Testn):
    def __init__(self, *args, **kwargs):
        super(Test0_Inv, self).__init__(*args, **kwargs)

    def test_duplicate_type(self):
        with self.assertRaises(ValueError, msg="Duplicate CDDL type found"):
            self.decode_file(p_test_vectors12[0], p_manifest12, p_manifest12)


class Test1(Testn):
    def __init__(self, *args, **kwargs):
        super(Test1, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors12[1], p_manifest12)

    def test_components(self):
        self.assertEqual(
            [b'\x00'],
            self.decoded.suit_manifest.suit_common.suit_components[0][0].bstr)

    def test_uri(self):
        self.assertEqual(
            "http://example.com/file.bin",
            self.decoded.suit_manifest.SUIT_Severable_Manifest_Members.suit_install[0].suit_install.union[0].SUIT_Directive.suit_directive_set_parameters.map[0].suit_parameter_uri)


class Test2(Testn):
    def __init__(self, *args, **kwargs):
        super(Test2, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors12[2], p_manifest12)

    def test_severed_uri(self):
        self.assertEqual(
            "http://example.com/very/long/path/to/file/file.bin",
            self.decoded.SUIT_Severable_Manifest_Members.suit_install[0].suit_install.union[0].SUIT_Directive.suit_directive_set_parameters.map[0].suit_parameter_uri)

    def test_severed_text(self):
        self.assertIn(
            "Example 2",
            self.decoded.SUIT_Severable_Manifest_Members.suit_text[0].suit_text.SUIT_Text_Keys.suit_text_manifest_description[0])
        self.assertEqual(
            [b'\x00'],
            self.decoded.SUIT_Severable_Manifest_Members.suit_text[0].suit_text.SUIT_Component_Identifier[0].SUIT_Component_Identifier_key.bstr)
        self.assertEqual(
            "arm.com",
            self.decoded.SUIT_Severable_Manifest_Members.suit_text[0].suit_text.SUIT_Component_Identifier[0].SUIT_Component_Identifier.SUIT_Text_Component_Keys.suit_text_vendor_domain[0])
        self.assertEqual(
            "This component is a demonstration. The digest is a sample pattern, not a real one.",
            self.decoded.SUIT_Severable_Manifest_Members.suit_text[0].suit_text.SUIT_Component_Identifier[0].SUIT_Component_Identifier.SUIT_Text_Component_Keys.suit_text_component_description[0])


class Test3(Testn):
    def __init__(self, *args, **kwargs):
        super(Test3, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors12[3], p_manifest12)

    def test_A_B_offset(self):
        self.assertEqual(
            33792,
            self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[1].SUIT_Common_Commands.suit_directive_try_each.SUIT_Directive_Try_Each_Argument.SUIT_Command_Sequence_bstr[0].union[0].SUIT_Directive.suit_directive_override_parameters.map[0].suit_parameter_component_offset)
        self.assertEqual(
            541696,
            self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[1].SUIT_Common_Commands.suit_directive_try_each.SUIT_Directive_Try_Each_Argument.SUIT_Command_Sequence_bstr[1].union[0].SUIT_Directive.suit_directive_override_parameters.map[0].suit_parameter_component_offset)


class Test4(Testn):
    def __init__(self, *args, **kwargs):
        super(Test4, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors12[4], p_manifest12)

    def test_load_decompress(self):
        self.assertEqual(
            0,
            self.decoded.suit_manifest.SUIT_Unseverable_Members.suit_load[0].suit_load.union[1].SUIT_Directive.suit_directive_set_parameters.map[3].suit_parameter_source_component)
        self.assertEqual(
            "SUIT_Compression_Algorithm_zlib",
            self.decoded.suit_manifest.SUIT_Unseverable_Members.suit_load[0].suit_load.union[1].SUIT_Directive.suit_directive_set_parameters.map[2].suit_parameter_compression_info.suit_compression_algorithm)


class Test5(Testn):
    def __init__(self, *args, **kwargs):
        super(Test5, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors12[5], p_manifest12)

    def test_two_image_match(self):
        self.assertEqual(
            "suit_condition_image_match",
            self.decoded.suit_manifest.SUIT_Severable_Manifest_Members.suit_install[0].suit_install.union[3].SUIT_Condition.union_choice)
        self.assertEqual(
            "suit_condition_image_match",
            self.decoded.suit_manifest.SUIT_Severable_Manifest_Members.suit_install[0].suit_install.union[7].SUIT_Condition.union_choice)


def dumps(obj):
    return cbor2.dumps(obj, canonical=True)


def loads(string):
    return cbor2.loads(string)


class Test6(Testn):
    def __init__(self, *args, **kwargs):
        super(Test6, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[0], p_manifest14, p_cose)

    def test_authentication(self):
        digest = bytes.fromhex("a6c4590ac53043a98e8c4106e1e31b305516d7cf0a655eddfac6d45c810e036a")
        signature = bytes.fromhex("d11a2dd9610fb62a707335f584079225709f96e8117e7eeed98a2f207d05c8ecfba1755208f6abea977b8a6efe3bc2ca3215e1193be201467d052b42db6b7287")
        sig_struct = bytes.fromhex("846a5369676e61747572653143a10126405820a6c4590ac53043a98e8c4106e1e31b305516d7cf0a655eddfac6d45c810e036a")
        # key = VerifyingKey.from_pem(p_manifest14_pub.read_text())
        # key.verify_digest(signature, digest)
        # key.verify(signature, digest, hashfunc=sha256)
        # key.verify(signature, sig_struct, hashfunc=sha256)

        self.assertEqual("COSE_Sign1_Tagged", self.decoded.suit_authentication_wrapper.SUIT_Authentication_Block_bstr[0].union_choice)
        self.assertEqual(-7, self.decoded.suit_authentication_wrapper.SUIT_Authentication_Block_bstr[0].COSE_Sign1_Tagged.Headers.protected.header_map_bstr.Generic_Headers.uint1union[0].int)
        manifest_signature = self.decoded.suit_authentication_wrapper.SUIT_Authentication_Block_bstr[0].COSE_Sign1_Tagged.signature
        sig_struct = ["Signature", self.decoded.suit_authentication_wrapper.SUIT_Authentication_Block_bstr[0].COSE_Sign1_Tagged.Headers.protected.header_map_bstr_bstr, b'', b'']
        sig_struct_encoded = dumps(sig_struct)
        # self.assertEqual(dumps(self.decoded.suit_manifest.orig_obj), self.decoded.orig_obj[3])
        # manifest_str = dumps(self.decoded.suit_manifest_bstr)
        # manifest_hash = sha256(manifest_str).digest()
        # manifest_hash = dumps(sha256(manifest_str).digest())
        # manifest_suit_digest = self.decoded.suit_authentication_wrapper.SUIT_Digest_bstr_bstr
        manifest_suit_digest = self.decoded.suit_authentication_wrapper.SUIT_Digest_bstr_bstr
        sig_struct_encoded = sig_struct_encoded[:-1] + (manifest_suit_digest)
        # sig_struct_encoded = sig_struct_encoded[:-1] + dumps(manifest_suit_digest)
        # sig_struct_encoded = sig_struct_encoded[:-1] + dumps(manifest_hash)
        # sig_struct_encoded = sig_struct_encoded[:-1] + dumps(manifest_suit_digest)
        # sig_struct_encoded = sig_struct_encoded[:-1] + dumps(dumps(manifest_suit_digest))
        # res = self.my_types["Sig_structure"].validate_str(sig_struct_encoded)
        # print (sig_struct_encoded.hex())
        loaded = loads(sig_struct_encoded)
        # key = VerifyingKey.from_pem(p_manifest14_pub.read_text())
        # print(sig_struct_encoded.hex())
        # print(key.to_string().hex())
        # print(manifest_signature.hex())
        # res = key.verify(manifest_signature, dumps(self.decoded.orig_obj[3]), hashfunc=sha256)
        # res = key.verify_digest(manifest_signature, manifest_hash)
        # res = key.verify(manifest_signature, manifest_hash, hashfunc=sha256)
        # res = key.verify(manifest_signature, dumps(manifest_hash), hashfunc=sha256)
        # res = key.verify(manifest_signature, manifest_suit_digest, hashfunc=sha256)
        # res = key.verify(manifest_signature, dumps(manifest_suit_digest), hashfunc=sha256)
        # res = key.verify(manifest_signature, dumps(sig_struct_encoded), hashfunc=sha256)
        # res = key.verify(manifest_signature, sig_struct_encoded, hashfunc=sha256)
        # print(res)


class Test7(Testn):
    def __init__(self, *args, **kwargs):
        super(Test7, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[1], p_manifest14, p_cose)

    def test_structure(self):
        self.assertEqual("COSE_Sign1_Tagged", self.decoded.suit_authentication_wrapper.SUIT_Authentication_Block_bstr[0].union_choice)
        self.assertEqual(-7, self.decoded.suit_authentication_wrapper.SUIT_Authentication_Block_bstr[0].COSE_Sign1_Tagged.Headers.protected.header_map_bstr.Generic_Headers.uint1union[0].int)
        self.assertEqual(bytes.fromhex("60c61d6eb7a1aaeddc49ce8157a55cff0821537eeee77a4ded44155b03045132"), self.decoded.suit_authentication_wrapper.SUIT_Digest_bstr.suit_digest_bytes)
        self.assertEqual(1, self.decoded.suit_manifest.suit_manifest_sequence_number)
        self.assertEqual(bytes.fromhex("fa6b4a53d5ad5fdfbe9de663e4d41ffe"), self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[0].SUIT_Common_Commands.suit_directive_override_parameters.map[0].suit_parameter_vendor_identifier.RFC4122_UUID)
        self.assertEqual(bytes.fromhex("1492af1425695e48bf429b2d51f2ab45"), self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[0].SUIT_Common_Commands.suit_directive_override_parameters.map[1].suit_parameter_class_identifier)
        self.assertEqual(bytes.fromhex("00112233445566778899aabbccddeeff0123456789abcdeffedcba9876543210"), self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[0].SUIT_Common_Commands.suit_directive_override_parameters.map[2].suit_parameter_image_digest.suit_digest_bytes)
        self.assertEqual('cose_alg_sha_256', self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[0].SUIT_Common_Commands.suit_directive_override_parameters.map[2].suit_parameter_image_digest.suit_digest_algorithm_id.union_choice)
        self.assertEqual(34768, self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[0].SUIT_Common_Commands.suit_directive_override_parameters.map[3].suit_parameter_image_size)
        self.assertEqual(4, len(self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[0].SUIT_Common_Commands.suit_directive_override_parameters.map))
        self.assertEqual(15, self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[1].SUIT_Condition.suit_condition_vendor_identifier.SUIT_Rep_Policy)
        self.assertEqual(15, self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[2].SUIT_Condition.suit_condition_class_identifier.SUIT_Rep_Policy)
        self.assertEqual(3, len(self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union))
        self.assertEqual(2, len(self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[0]))
        self.assertEqual(2, len(self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[0].SUIT_Common_Commands))
        self.assertEqual(1, len(self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[0].SUIT_Common_Commands.suit_directive_override_parameters))
        self.assertEqual(4, len(self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[0].SUIT_Common_Commands.suit_directive_override_parameters.map))
        self.assertEqual(2, len(self.decoded.suit_manifest.suit_common.suit_common_sequence[0].suit_common_sequence.union[0].SUIT_Common_Commands.suit_directive_override_parameters.map[0]))

    def test_cbor_pen(self):
        data = bytes.fromhex(p_test_vectors14[1].read_text().replace("\n", ""))
        struct = loads(data)
        struct2 = loads(struct.value[3])  # manifest
        struct3 = loads(struct2[3])  # common sequence
        struct4 = loads(struct3[4])  # override params
        self.assertEqual(struct4[0], 20)
        self.assertTrue(isinstance(struct4[1][1], bytes))
        struct4[1][1] = cbor2.CBORTag(112, struct4[1][1])  # Add the tag for cbor-pen
        struct3[4] = dumps(struct4)
        struct2[3] = dumps(struct3)
        struct.value[3] = dumps(struct2)
        data = dumps(struct)
        self.decode_string(data, p_manifest14, p_cose)


class Test7Inv(Testn):
    def test_inv0(self):
        data = bytes.fromhex(p_test_vectors14[1].read_text().replace("\n", ""))
        struct = loads(data)
        struct2 = loads(struct.value[2])  # authentication
        struct3 = loads(struct2[1])
        struct3.tag = 99999  # invalid tag for COSE_Sign1
        struct2[1] = dumps(struct3)
        struct.value[2] = dumps(struct2)
        data = dumps(struct)
        try:
            self.decode_string(data, p_manifest14, p_cose)
        except zcbor.CddlValidationError as e:
            return
        else:
            assert False, "Should have failed validation"

    def test_inv1(self):
        data = bytes.fromhex(p_test_vectors14[1].read_text().replace("\n", ""))
        struct = loads(data)
        struct2 = loads(struct.value[3])  # manifest
        struct2[1] += 1  # invalid manifest version
        struct.value[3] = dumps(struct2)
        data = dumps(struct)
        try:
            self.decode_string(data, p_manifest14, p_cose)
        except zcbor.CddlValidationError as e:
            return
        else:
            assert False, "Should have failed validation"

    def test_inv2(self):
        data = bytes.fromhex(p_test_vectors14[1].read_text().replace("\n", ""))
        struct = loads(data)
        struct.value[23] = b''  # Invalid integrated payload key
        data = dumps(struct)
        try:
            self.decode_string(data, p_manifest14, p_cose)
        except (zcbor.CddlValidationError, cbor2.CBORDecodeEOF) as e:
            return
        else:
            assert False, "Should have failed validation"

    def test_inv3(self):
        data = bytes.fromhex(p_test_vectors14[1].read_text().replace("\n", ""))
        struct = loads(data)
        struct2 = loads(struct.value[3])  # manifest
        struct3 = loads(struct2[3])  # common sequence
        struct4 = loads(struct3[4])  # override params
        self.assertEqual(struct4[0], 20)
        self.assertTrue(isinstance(struct4[1][1], bytes))
        struct4[1][1] += b'x'  # vendor ID: wrong length
        struct3[4] = dumps(struct4)
        struct2[3] = dumps(struct3)
        struct.value[3] = dumps(struct2)
        data = dumps(struct)
        try:
            self.decode_string(data, p_manifest14, p_cose)
        except zcbor.CddlValidationError as e:
            return
        else:
            assert False, "Should have failed validation"


class Test8(Testn):
    def __init__(self, *args, **kwargs):
        super(Test8, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[2], p_manifest14, p_cose)

    def test_text(self):
        self.assertEqual(
            bytes.fromhex('2bfc4d0cc6680be7dd9f5ca30aa2bb5d1998145de33d54101b80e2ca49faf918'),
            self.decoded.suit_manifest.SUIT_Severable_Members_Choice.suit_text[0].SUIT_Digest.suit_digest_bytes)
        self.assertEqual(
            bytes.fromhex('2bfc4d0cc6680be7dd9f5ca30aa2bb5d1998145de33d54101b80e2ca49faf918'),
            sha256(dumps(self.decoded.SUIT_Severable_Manifest_Members.suit_text[0].suit_text_bstr)).digest())
        self.assertEqual('arm.com', self.decoded.SUIT_Severable_Manifest_Members.suit_text[0].suit_text.SUIT_Component_Identifier[0].SUIT_Component_Identifier.SUIT_Text_Component_Keys.suit_text_vendor_domain[0])
        self.assertEqual('This component is a demonstration. The digest is a sample pattern, not a real one.', self.decoded.SUIT_Severable_Manifest_Members.suit_text[0].suit_text.SUIT_Component_Identifier[0].SUIT_Component_Identifier.SUIT_Text_Component_Keys.suit_text_component_description[0])

        # Check manifest description. The concatenation and .replace() call are there to add
        # trailing whitespace to all blank lines except the first.
        # This is done in this way to avoid editors automatically removing the whitespace.
        self.assertEqual('''## Example 2: Simultaneous Download, Installation, Secure Boot, Severed Fields
''' + '''
    This example covers the following templates:

    * Compatibility Check ({{template-compatibility-check}})
    * Secure Boot ({{template-secure-boot}})
    * Firmware Download ({{firmware-download-template}})

    This example also demonstrates severable elements ({{ovr-severable}}), and text ({{manifest-digest-text}}).'''.replace("\n\n", "\n    \n"), self.decoded.SUIT_Severable_Manifest_Members.suit_text[0].suit_text.SUIT_Text_Keys.suit_text_manifest_description[0])


class Test9(Testn):
    def __init__(self, *args, **kwargs):
        super(Test9, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[3], p_manifest14, p_cose)

    def test_try_each(self):
        self.assertEqual(2, len(self.decoded.suit_manifest.SUIT_Severable_Members_Choice.suit_install[0].SUIT_Command_Sequence_bstr.union[0].SUIT_Directive.suit_directive_try_each.SUIT_Directive_Try_Each_Argument.SUIT_Command_Sequence_bstr))
        self.assertEqual(33792, self.decoded.suit_manifest.SUIT_Severable_Members_Choice.suit_install[0].SUIT_Command_Sequence_bstr.union[0].SUIT_Directive.suit_directive_try_each.SUIT_Directive_Try_Each_Argument.SUIT_Command_Sequence_bstr[0].union[0].SUIT_Directive.suit_directive_override_parameters.map[0].suit_parameter_component_slot)
        self.assertEqual(541696, self.decoded.suit_manifest.SUIT_Severable_Members_Choice.suit_install[0].SUIT_Command_Sequence_bstr.union[0].SUIT_Directive.suit_directive_try_each.SUIT_Directive_Try_Each_Argument.SUIT_Command_Sequence_bstr[1].union[0].SUIT_Directive.suit_directive_override_parameters.map[0].suit_parameter_component_slot)


class Test10(Testn):
    def __init__(self, *args, **kwargs):
        super(Test10, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[4], p_manifest14, p_cose)

    def test_components(self):
        self.assertEqual(3, len(self.decoded.suit_manifest.suit_common.suit_components[0]))
        self.assertEqual(b'\x00', self.decoded.suit_manifest.suit_common.suit_components[0][0].bstr[0])
        self.assertEqual(b'\x02', self.decoded.suit_manifest.suit_common.suit_components[0][1].bstr[0])
        self.assertEqual(b'\x01', self.decoded.suit_manifest.suit_common.suit_components[0][2].bstr[0])


class Test11(Testn):
    def __init__(self, *args, **kwargs):
        super(Test11, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[5], p_manifest14, p_cose)

    def test_validate(self):
        self.assertEqual(4, len(self.decoded.suit_manifest.SUIT_Unseverable_Members.suit_validate[0].suit_validate.union))
        self.assertEqual(15, self.decoded.suit_manifest.SUIT_Unseverable_Members.suit_validate[0].suit_validate.union[1].SUIT_Condition.suit_condition_image_match.SUIT_Rep_Policy)


class Test11Inv(Testn):
    def test_invalid_rep_policy(self):
        data = bytes.fromhex(p_test_vectors14[5].read_text().replace("\n", ""))
        struct = loads(data)
        struct2 = loads(struct.value[3])  # manifest
        struct3 = loads(struct2[10])  # suit_validate
        struct3[3] += 16  # invalid Rep_Policy
        struct2[10] = dumps(struct3)
        struct.value[3] = dumps(struct2)
        data = dumps(struct)
        try:
            self.decode_string(data, p_manifest14, p_cose)
        except zcbor.CddlValidationError as e:
            return
        else:
            assert False, "Should have failed validation"


class Test12(Test6):
    def __init__(self, *args, **kwargs):
        super(Test6, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[0], p_manifest16, p_cose)


class Test13(Test7):
    def __init__(self, *args, **kwargs):
        super(Test7, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[1], p_manifest16, p_cose)


class Test13Inv(Test7Inv):
    def __init__(self, *args, **kwargs):
        super(Test7Inv, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[1], p_manifest16, p_cose)


class Test14(Test8):
    def __init__(self, *args, **kwargs):
        super(Test8, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[2], p_manifest16, p_cose)


class Test15(Test9):
    def __init__(self, *args, **kwargs):
        super(Test9, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[3], p_manifest16, p_cose)


# Comment out because example 4 uses compression which is unsupported in manifest16
# class Test16(Test10):
#     def __init__(self, *args, **kwargs):
#         super(Test10, self).__init__(*args, **kwargs)
#         self.decode_file(p_test_vectors14[4], p_manifest16, p_cose)


class Test17(Test11):
    def __init__(self, *args, **kwargs):
        super(Test11, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[5], p_manifest16, p_cose)


class Test17Inv(Test11Inv):
    def __init__(self, *args, **kwargs):
        super(Test11Inv, self).__init__(*args, **kwargs)
        self.decode_file(p_test_vectors14[5], p_manifest16, p_cose)


class TestCLI(TestCase):
    def get_std_args(self, input):
        return ["zcbor", "--cddl", str(p_manifest12), "--default-max-qty", "16", "convert", "--input", str(input), "-t", "SUIT_Envelope_Tagged"]

    def do_testn(self, n):
        call0 = Popen(self.get_std_args(p_test_vectors12[n]) + ["--output", "-", "--output-as", "cbor"], stdout=PIPE)
        stdout0, _ = call0.communicate()
        self.assertEqual(0, call0.returncode)

        call1 = Popen(self.get_std_args("-") + ["--input-as", "cbor", "--output", "-", "--output-as", "json"], stdin=PIPE, stdout=PIPE)
        stdout1, _ = call1.communicate(input=stdout0)
        self.assertEqual(0, call1.returncode)

        call2 = Popen(self.get_std_args("-") + ["--input-as", "json", "--output", "-", "--output-as", "yaml"], stdin=PIPE, stdout=PIPE)
        stdout2, _ = call2.communicate(input=stdout1)
        self.assertEqual(0, call2.returncode)

        call3 = Popen(self.get_std_args("-") + ["--input-as", "yaml", "--output", "-", "--output-as", "cbor"], stdin=PIPE, stdout=PIPE)
        stdout3, _ = call3.communicate(input=stdout2)
        self.assertEqual(0, call3.returncode)

        self.assertEqual(stdout0, stdout3)

        call4 = Popen(self.get_std_args("-") + ["--input-as", "cbor", "--output", "-", "--output-as", "cborhex"], stdin=PIPE, stdout=PIPE)
        stdout4, _ = call4.communicate(input=stdout3)
        self.assertEqual(0, call4.returncode)

        call5 = Popen(self.get_std_args("-") + ["--input-as", "cborhex", "--output", "-", "--output-as", "json"], stdin=PIPE, stdout=PIPE)
        stdout5, _ = call5.communicate(input=stdout4)
        self.assertEqual(0, call5.returncode)

        self.assertEqual(stdout1, stdout5)

        self.maxDiff = None

        with open(p_test_vectors12[n], 'r') as f:
            self.assertEqual(sub(r"\W+", "", f.read()), sub(r"\W+", "", stdout4.decode("utf-8")))

    def test_0(self):
        self.do_testn(0)

    def test_1(self):
        self.do_testn(1)

    def test_2(self):
        self.do_testn(2)

    def test_3(self):
        self.do_testn(3)

    def test_4(self):
        self.do_testn(4)

    def test_5(self):
        self.do_testn(5)

    def test_map_bstr(self):
        args = ["zcbor", "--cddl", str(p_map_bstr_cddl), "convert", "--input", str(p_map_bstr_yaml), "-t", "map", "--output", "-"]
        call1 = Popen(args, stdout=PIPE)
        stdout1, _ = call1.communicate()
        self.assertEqual(0, call1.returncode)
        self.assertEqual(dumps({"test": bytes.fromhex("1234abcd"), "test2": cbor2.CBORTag(1234, bytes.fromhex("1a2b3c4d")), ("test3",): dumps(1234)}), stdout1)

    def test_decode_encode(self):
        args = ["zcbor", "--cddl", str(p_map_bstr_cddl), "code", "-d", "-e", "-t", "map"]
        call1 = Popen(args, stdout=PIPE, stderr=PIPE)

        _, stderr1 = call1.communicate()
        self.assertNotEqual(0, call1.returncode)
        self.assertIn(b"error: Please specify exactly one of --decode or --encode", stderr1)


class TestOptional(TestCase):
    def test_0(self):
        with open(p_optional, 'r') as f:
            cddl_res = zcbor.DataTranslator.from_cddl(f.read(), 16)
        cddl = cddl_res.my_types['cfg']
        test_yaml = """
            mem_config:
                - 0
                - 5"""
        decoded = cddl.decode_str_yaml(test_yaml)
        self.assertEqual(decoded.mem_config[0].READ.union_choice, "uint0")
        self.assertEqual(decoded.mem_config[0].N, [5])


if __name__ == "__main__":
    main()
