# zcbor v. 0.9.99

* The fragmented payload API has been completely redesigned to accomodate adding the encoding counterpart.
  The docs have been updated and there's a new section in the README to explain the functionality.

  * You must now define ZCBOR_FRAGMENTS to access the API
  * `zcbor_*str_decode_fragment()` has been renamed to `zcbor_*str_fragments_start_decode()`
  * After calling `zcbor_*str_fragments_start_decode()`, you must now retrieve the first fragment manually with `zcbor_str_fragment_decode()`, instead of via an argument.
  * `zcbor_next_fragment()` and `zcbor_bstr_next_fragment()` have merged and is now called `zcbor_str_fragment_decode()`.
    It does not take a `prev_fragment` argument, instead, this state is kept internally in the state struct.
  * `zcbor_bstr_start_decode_fragment()` has been renamed to `zcbor_cbor_bstr_fragments_start_decode()` and does not return a fragment.
    To retrieve fragments when decoding a CBOR-encoded bstr, use `zcbor_str_fragment_decode()`


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
