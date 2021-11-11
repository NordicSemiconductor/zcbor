# cddl-gen v. 0.3.1

__cddl-gen has been renamed to [zcbor](https://pypi.org/project/zcbor/)__
This version of the pip package is deprecated.

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
