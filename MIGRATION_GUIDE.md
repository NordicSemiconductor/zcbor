# zcbor v. 0.9.99

* `from_cddl()` and `__init__()` now takes only keyword-only arguments.
  Positional arguments to any constructor or `from_cddl()` function must now be made into keyword arguments.
  Arguments have not changed otherwise.
  This was done to avoid ambiguity to do with positional arguments, and generally simplify passing arguments around inside zcbor.

* [Recommended] A new macro `ZCBOR_CAST_FP` has been added for casting function pointers for use in the zcbor API.
  The macro will first check that the function pointer has one of the supported signatures.
  You are recommended to use it on any function pointer that is passed as an argument to a zcbor function.
  See the macro documentation in zcbor_common.h for more info.

* Code generation:

  * Integers whose values are known to be within 8 or 16 bytes now use the corresponding integer types (`uint8_t`/`int8_t`/`uint16_t`/`int16_t`) instead of larger types.
    In certain specific cases, the type of an argument to a `cbor_decode_*` or `cbor_decode_*` can change when regenerating the code, requiring changes in your non-generated code.
    More commonly, struct members will change to use smaller int types.


# zcbor v. 0.9.0

* `zcbor_simple_*()` functions have been removed to avoid confusion about their use.
  They are still in the C file because they are used by other functions.
  Instead, use the specific functions for the currently supported simple values, i.e.
  `zcbor_bool_*()`, `zcbor_nil_*()`, and `zcbor_undefined_*()`.
  If a removed variant is strictly needed, add your own forward declaration in your code.

* Code generation naming:

  * More C keywords are now capitalized to avoid naming collision.
    You might have to capitalize some instances if your code was generated to have those names.

  * A fix was made to the naming of bstr elements with a .size specifier, which might mean that these elements change name in your code when you regenerate.


# zcbor v. 0.8.0

(copied after the fact from [Zephyr 3.6.0 migration guide](https://github.com/zephyrproject-rtos/zephyr/blob/v3.6.0/doc/releases/migration-guide-3.6.rst))

* If you have zcbor-generated code that relies on the zcbor libraries through Zephyr, you must
  regenerate the files using zcbor 0.8.1. Note that the names of generated types and members has
  been overhauled, so the code using the generated code must likely be changed.
  For example:

  * Leading single underscores and all double underscores are largely gone,
  * Names sometimes gain suffixes like ``_m`` or ``_l`` for disambiguation.
  * All enum (choice) names have now gained a ``_c`` suffix, so the enum name no longer matches
    the corresponding member name exactly (because this broke C++ namespace rules).

* The function `zcbor_new_state`, `zcbor_new_decode_state` and the macro
  `ZCBOR_STATE_D` have gained new parameters related to decoding of unordered maps.
  Unless you are using that new functionality, these can all be set to NULL or 0.

* The functions `zcbor_bstr_put_term` and `zcbor_tstr_put_term` have gained a new
  parameter ``maxlen``, referring to the maximum length of the parameter ``str``.
  This parameter is passed directly to `strnlen` under the hood.

* The function `zcbor_tag_encode` has been renamed to `zcbor_tag_put`.

* Printing has been changed significantly, e.g. `zcbor_print` is now called
  `zcbor_log`, and `zcbor_trace` with no parameters is gone, and in its place are
  `zcbor_trace_file` and `zcbor_trace`, both of which take a ``state`` parameter.
