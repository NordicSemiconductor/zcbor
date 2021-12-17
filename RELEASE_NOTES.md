# zcbor v. 0.3.99

**cddl-gen has been renamed to zcbor!**

Any new bugs, requests, or missing features should be reported as [Github issues](https://github.com/NordicSemiconductor/zcbor/issues).

## Improvements:

### Command line
 * Rename script to zcbor.
 * Better 64 bit support:
   * New argument: `--default-bit-size` which controls the assumed size of integers. See zcbor code `--help` for more information.

### Command line
 * Rename module to zcbor.

### C Libraries
 * All public function names have been prefixed with `zcbor_`.
 * Many function names have also been changed slightly.
 * Better API docs.
 * Better 64 bit support:
   * New 64 bit integer decoding and encoding functions (in addition to existing 32 bit functions).
   * `uint32_t` usage has been largely transitioned to `size_t` or `uint_fast32_t` which are typically the same size as the architecture (32 or 64 bits).
   * This means that list/map/string sizes can be 64 bits on 64 bit architectures. Note that having 64 bit sizes on 32 bit architectures rarely make sense since they won't fit in the address space.
 * New helper macros for instantiating state variables: `ZCBOR_STATE_D()` and `ZCBOR_STATE_E()` for decoding and encoding respectively.
 * Split `zcbor_multi_encode()` into `zcbor_multi_encode()` and `zcbor_multi_encode_minmax()`. The former is more useful for manual use, while the latter is for generated code.
 * Remove `zcbor_any_encode()` since it was just an alias for `zcbor_nil_put()`.
 * Rename `zcbor_string_type_t` to `struct zcbor_string`.
 * Rename `zcbor_state_backups_t` to `struct zcbor_state_constant`.

### Tests
 * decode: Remove test4 since it hasn't been used or updated.
 * New unit test directory and project. Tests:
   * 64 bit vs 32 bit integers.
   * 64 bit vs 32 bit strings.
   * String encoding and decoding macros (`_lit()`, `_arr()`, `_term()`).
 * Collect boilerplate Cmake code for tests into a new file test_template.cmake.
 * Add tests for draft-ietf-suit-manifest-16.


## Bugfixes:
 * zcbor.py: Avoid #include-ing the full path of the type file.
 * zcbor.py: Avoid creating functions with "None" result type.
 * src: Fix code so it builds correctly with asserts enabled.


## Unsupported CDDL features

Not all features outlined in the [CDDL spec](https://datatracker.ietf.org/doc/html/rfc8610) are supported by zcbor.
The following is a list of limitiations and missing features:

 * Map elements in data must appear in the same order as they appear in the CDDL.
 * C API doesn't support searching through a map.
 * Floating point numbers.
 * `undefined`.
 * Using `&()` to turn groups into choices (unions). `&()` is supported when used with `.bits`.
 * Representation Types (`#x.y`), except for tags (`#6.y(foo)`) which are supported.
 * Unwrapping (`~`)
 * The control operator `.regexp`.
 * The control operator `.ne`.
 * The control operator `.default`.
 * Generics (`foo<a, b>`).
 * Not all types from the "standard prelude" are supported.
 * Most of the "Extended Diagnostic Notation" is unsupported.


# cddl-gen v. 0.3.0

Any new bugs, requests, or missing features should be reported as [Github issues](https://github.com/NordicSemiconductor/cddl-gen/issues).

## Improvements:

### CDDL language
 * Add more support for hexadecimal/octal/binary numbers in CDDL. E.g. in size ranges or value limits
 * Add support for ranges with three dots in CDDL files
 * Change CDDL quantifier syntax from `x**y` to `x*y`
 * Add support for `$$` and `$`
 * Add support for the `.bits` operator.

### Command line
 * Change command line name from `cddl_gen` to `cddl-gen`
 * Add `--dq` as a shorthand for `--default-max-qty`
 * Allow passing multiple cddl files via command line
 * Allow adding the current git sha to generated file headers.
 * Various changes to the logic for naming in code generation
   * This can give different names of e.g. struct members compared with previous versions of cddl-gen
 * Generate a cmake file when generating code.
   * The `target_cddl_source()` Cmake function has been removed.
   * See README.md for more info.

### Python module
 * Improve tags support in DataTranslator
 * Improve decoding of cbor bstrs in DataTranslator
   * Provide the raw string along with the decoded object.

### C Libraries
 * Refactor cbor_common.c
 * Add support for indeterminate length arrays in cbor_decode.c.

### Tests
 * Move all cddl files into tests/cases
 * Run tests with twister instead of custom bash scripts
 * Add tests for draft-ietf-suit-manifest-14


## Bugfixes:
 * cbor_encode.h: Add missing `.` in `tstrx_put` and `tstrx_put_term`
 * cbor_encode.c: Fix `get_result_len` and add big-endian support
 * cddl_gen.py: Fix bug where range checks were dropped
 * cbor_decode.c: Fix bug where a `nil` could be interpreted as `true`
 * cddl_gen.py: Fix parsing of ranges with no max (`n*` or `n**`)
 * cbor_encode.c: Fix bug where minimum quantity restrictions were ignored
 * cddl_gen: Fix generation of map-within-map
 * cbor_decode.c: Add overflow checks and fix assert in `any_decode()`.


## Unsupported CDDL features

Not all features outlined in the [CDDL spec](https://datatracker.ietf.org/doc/html/rfc8610) are supported by cddl-gen.
The following is a list of limitiations and missing features:

 * Map elements in data must appear in the same order as they appear in the CDDL.
 * Floating point numbers.
 * `undefined`.
 * Using `&()` to turn groups into choices (unions). `&()` is supported when used with `.bits`.
 * Representation Types (`#x.y`), except for tags (`#6.y(foo)`) which are supported.
 * Unwrapping (`~`)
 * The control operator `.regexp`.
 * The control operator `.ne`.
 * The control operator `.default`.
 * Generics (`foo<a, b>`).
 * Not all types from the "standard prelude" are supported.
 * Most of the "Extended Diagnostic Notation" is unsupported.
