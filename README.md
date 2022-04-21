zcbor
=====

zcbor is a low footprint [CBOR](https://en.wikipedia.org/wiki/CBOR) library in the C language that comes with a schema-driven script tool that can validate your data, or even generate code for you.
Aside from the script, the CBOR library is a standalone library which is tailored for use in microcontrollers.

The validation part of the script can also work with YAML and JSON data.
It can for example validate a YAML file against a schema and convert it into CBOR.

The schema language used by zcbor is CDDL (Consise Data Definition Language) which is a powerful human-readable data description language defined in [IETF RFC 8610](https://datatracker.ietf.org/doc/rfc8610/).

zcbor was previously called "cddl-gen".

CBOR decoding/encoding library
==============================

The CBOR library found at [headers](include) and [source](src) is used by the generated code, but can also be used directly.
If so, you must instantiate a `zcbor_state_t` object, which is most easily done using the `zcbor_new_*_state()` functions or the `ZCBOR_STATE_*()` macros.

The `elem_count` member refers to the number of encoded objects in the current list or map.
`elem_count` starts again when entering a nested list or map, and is restored when exiting.

`elem_count` is one reason for needing "backup" states (the other is to allow rollback of the payload).
You need a number of backups corresponding to the maximum number of nested levels in your data.

Backups are needed for encoding if you are using canonical encoding (`ZCBOR_CANONICAL`), or using the `bstrx_cbor_*` functions.
Backups are needed for decoding if there are any lists, maps, or CBOR-encoded strings in the data.

Note that the benefits of using the library directly is greater for encoding than for decoding.
For decoding, the code generation will provide a number of checks that are tedious to write manually, and easy to forget.

```c
/** The number of states must be at least equal to one more than the maximum
 *  nested depth of the data.
 */
zcbor_state_t states[n];

/** Initialize the states. After calling this, states[0] is ready to be used
 *  with the encoding/decoding APIs.
 *  elem_count must be the maximum expected number of top-level elements when
 *  decoding (1 if the data is wrapped in a list).
 *  When encoding, elem_count must be 0.
 */
zcbor_new_state(states, n, payload, payload_len, elem_count);

/** Alternatively, use one of the following convenience macros. */
ZCBOR_STATE_D(decode_state, n, payload, payload_len, elem_count);
ZCBOR_STATE_E(encode_state, n, payload, payload_len, 0);
```

The CBOR libraries assume little-endianness by default, but you can define ZCBOR_BIG_ENDIAN to change this.

Configuration
-------------

The C library has a few compile-time configuration options.
These configuration options can be enabled by adding them as compile definitions to the build.

Name                      | Description
------------------------- | -----------
`ZCBOR_CANONICAL`         | When encoding lists and maps, do not use indefinite length encoding. Enabling `ZCBOR_CANONICAL` increases code size and makes the encoding library more often use state backups.
`ZCBOR_VERBOSE`           | Print messages on encoding/decoding errors (`zcbor_print()`), and also a trace message (`zcbor_trace()`) for each decoded value, and in each generated function (when using code generation). Requires `printk` as found in Zephyr.
`ZCBOR_ASSERTS`           | Enable asserts (`zcbor_assert()`). When they fail, the assert statements instruct the current function to return a `ZCBOR_ERR_ASSERTION` error. If `ZCBOR_VERBOSE` is enabled, a message is printed.
`ZCBOR_STOP_ON_ERROR`     | Enable the `stop_on_error` functionality. This makes all functions abort their execution if called when an error has already happened.
`ZCBOR_BIG_ENDIAN`        | All decoded values are returned as big-endian.

Schema-driven data manipulation and code generation
===================================================

By invoking `zcbor` (when installed via Pip or setup.py), or the Python script [zcbor.py](zcbor/zcbor.py) directly, you can generate C code that validates/encodes/decodes CBOR data conforming to a CDDL schema.
zcbor can also validate and convert CBOR data to and from JSON/YAML, either from the command line, or imported as a module.
Finally, the package contains a light-weight CBOR encoding/decoding library in C.
This library is used by the generated code, and can also be used directly in your own code.

Features
========

Here are some possible ways zcbor can be used:

 - Python scripting:
   - Validate a YAML file and translate it into CBOR e.g. for transmission.
   - Validate a YAML/JSON/CBOR file before processing it with some other tool
   - Decode and validate incoming CBOR data into human-readable YAML/JSON.
   - As part of a python script that processes YAML/JSON/CBOR files. zcbor is compatible with PyYAML and can additionally provide validation and/or easier inspection via named tuples.
 - C code:
   - Generate C code for validating and decoding or encoding CBOR, for use in optimized or constrained environments, such as microcontrollers.
   - Provide a low-footprint CBOR decoding/encoding library similar to TinyCBOR/QCBOR/NanoCBOR.

Python scripting
================

Invoking zcbor.py from the command line
---------------------------------------

The zcbor.py script can directly read CBOR, YAML, or JSON data and validate it against a CDDL description.
It can also freely convert the data between CBOR/YAML/JSON.
It can also output the data to a C file formatted as a byte array.

The following is a generalized example for converting (and validating) data from the command line.
The script infers the data format from the file extension, but the format can also be specified explicitly.
See `zcbor convert --help` for more information.

```sh
python3 <zcbor base>/zcbor/zcbor.py -c <CDDL description file> convert -t <which CDDL type to expect> -i <input data file> -o <output data file>
```

Or invoke its command line executable (if installed via `pip`):

```sh
zcbor -c <CDDL description file> convert -t <which CDDL type to expect> -i <input data file> -o <output data file>
```

Note that since CBOR supports more data types than YAML and JSON, zcbor uses an idiomatic format when converting to/from YAML/JSON.
This is relevant when handling YAML/JSON conversions of data that uses the unsupported features.
The following data types are supported by CBOR, but not by YAML (and JSON which is a subset of YAML):

 1. bytestrings: YAML supports only text strings. In YAML, bytestrings ('<bytestring>') are represented as {"bstr": "<hex-formatted bytestring>"}, or as {"bstr": <any type>} if the CBOR bytestring contains CBOR-formatted data, in which the data is decoded into <any type>.
 2. map keys other than text string: In YAML, such key value pairs are represented as {"keyval<unique int>": {"key": <key, not text>, "val": <value>}}
 3. tags: In cbor2, tags are represented by a special type, cbor2.CBORTag. In YAML, these are represented as {"tag": <tag number>, "val": <tagged data>}.

Importing zcbor in a Python script
----------------------------------

Importing zcbor gives access to the DataTranslator class which is used to implement the command line conversion features.
DataTranslator can be used to programmatically perform the translations, or to manipulate the data.
When accessing the data, you can choose between two internal formats:

 1. The format provided by the cbor2, yaml (pyyaml), and json packages.
    This is a format where the serialization types (map, list, string, number etc.) are mapped directly to the corresponding Python types.
    This format is common between these packages, which makes translation very simple.
    When returning this format, DataTranslator hides the idiomatic representations for bytestrings, tags, and non-text keys described above.
 2. A custom format which allows accessing the data via the names from the CDDL description file.
    This format is implemented using named tuples, and is immutable, meaning that it can be used for inspecting data, but not for changing or creating data.

Code generation
===============

The generated code consists of:
 - A header file containing typedefs for the types defined in the CDDL, as well as declarations for decoding functions for some types (those specified as entry types). The typedefs are the same for both encoding and decoding.
 - A C file containing all the encoding/decoding code.
   The code is split across multiple functions, and each function contains a single `if` statement which "and"s and "or"s together calls into the cbor libraries or to other generated decoding functions.

CDDL allows placing restrictions on the members of your data structure.
Restrictions can be on type, on content (e.g. values/sizes of ints or strings), and repetition (e.g. the number of members in a list).
The generated code will validate the input (i.e. the structure if encoding, or the payload for decoding), which means that it will check all the restriction set in the CDDL description, and fail if a restriction is broken.

The cbor libraries do most of the actual translation and moving of bytes, and the validation of values.

There are tests for the code generation in [tests/](tests/).
The tests require [Zephyr](https://github.com/zephyrproject-rtos/zephyr) (if your shell is set up to build Zephyr samples, the tests should also build).

Build system
------------

When calling zcbor with the argument `--output-cmake <file path>`, a cmake file will be created at that location.
The cmake file creates a cmake target and adds the generated and non-generated source files, and the include directories to the header files.
This cmake file can then be included in your project's `CMakeLists.txt` file, and the target can be linked into your project.
This is demonstrated in the tests, e.g. at tests/decode/test3_simple/CMakeLists.txt.
zcbor can be instructed to copy the non-generated sources to the same location as the generated sources with `--copy-sources`.


Introduction to CDDL
====================

In CDDL you define types from other types.
Types can be defined from base types, or from other types you define.
Types are declared with '`=`', e.g. `Foo = int` which declares the type `Foo` to be an integer, analogous to `typedef int Foo;` in C.
CDDL defines the following base types (this is not an exhaustive list):

 - `int`: Positive or negative integer
 - `uint`: Positive integer
 - `bstr`: Byte string
 - `tstr`: Text string
 - `bool`: Boolean
 - `nil`: Nil/Null value
 - `float`: Floating point value
 - `any`: Any single element

CDDL allows creating aggregate types:

 - `[]`: List. Elements don't need to have the same type.
 - `{}`: Map. Key/value pairs as are declared as `<key> => <value>` or `<key>: <value>`. Note that `:` is also used for labels.
 - `()`: Groups. Grouping with no enclosing type, which means that e.g. `Foo = [(int, bstr)]` is equivalent to `Foo = [int, bstr]`.
 - `/`: Unions. Analogous to unions in C. E.g. `Foo = int/bstr/Bar` where Foo is either an int, a bstr, or Bar (some custom type).

Literals can be used instead of the base type names:

 - Number: `Foo = 3`, where Foo is a uint with the additional requirement that it must have the value 3.
 - Number range: `Foo = -100..100`, where Foo is an int with value between -100 and 100.
 - Text string: `Foo = "hello"`, where Foo is a tstr with the requirement that it must be "hello".
 - True/False: `Foo = false`, where Foo is a bool which is always false.

Base types can also be restricted in other ways:

 - `.size`: Works for integers and strings. E.g. `Foo = uint .size 4` where Foo is a uint exactly 4 bytes long.
 - `.cbor`/`.cborseq`: E.g. `Foo = bstr .cbor Bar` where Foo is a bstr whose contents must be CBOR data decodeable as the Bar type.

An element can be repeated:

 - `?`: 0 or 1 time. E.g. `Foo = [int, ?bstr]`, where Foo is a list with an int possibly followed by a bstr.
 - `*`: 0 or more times. E.g. `Foo = [*tstr]`, where Foo is a list containing 0 or more tstrs.
 - `+`: 1 or more times. E.g. `Foo = [+Bar]`.
 - `x*y`: Between x and y times, inclusive. E.g. `Foo = {4*8(int => bstr)}` where Foo is a map with 4 to 8 key/value pairs where each key is an int and each value is a bstr.

Note that in the zcbor script and its generated code, the number of entries supported via `*` and `+` is affected by the default_max_qty value.

Any element can be labeled with `:`.
The label is only for readability and does not impact the data structure in any way.
E.g. `Foo = [name: tstr, age: uint]` is equivalent to `Foo = [tstr, uint]`.

See [test3_simple](tests/decode/test3_simple/) for CDDL example code.

Introduction to CBOR
====================

CBOR's format is described well on [Wikipedia](https://en.wikipedia.org/wiki/CBOR), but here's a synopsis:

Encoded CBOR data elements look like this.

```
| Header                       | Value                  | Payload                   |
| 1 byte                       | 0, 1, 2, 4, or 8 bytes | 0 - 2^64-1 bytes/elements |
| 3 bits     | 5 bits          |
| Major Type | Additional Info |
```

The available major types can be seen in `zcbor_major_type_t`.

For all major types, Values 0-23 are encoded directly in the _Additional info_, meaning that the _Value_ field is 0 bytes long.
If _Additional info_ is 24, 25, 26, or 27, the _Value_ field is 1, 2, 4, or 8 bytes long, respectively.

Major types `pint`, `nint`, `tag`, and `prim` elements have no payload, only _Value_.

 * `pint`: Interpret the _Value_ as a positive integer.
 * `nint`: Interpret the _Value_ as a positive integer, then multiply by -1 and subtract 1.
 * `tag`: The _Value_ says something about the next non-tag element.
   See the [CBOR tag documentation](See https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml) for details.
 * `prim`: Different _Additional info_ mean different things:
    * 20: `false`
    * 21: `true`
    * 22: `null`
    * 23: `undefined`
    * 25: Interpret the _Value_ as an IEEE 754 float16.
    * 26: Interpret the _Value_ as an IEEE 754 float32.
    * 27: Interpret the _Value_ as an IEEE 754 float64.
    * 31: End of an indefinite-length `list` or `map`.

For `bstr`, `tstr`, `list`, and `map`, the _Value_ describes the length of the _Payload_.
For `bstr` and `tstr`, the length is in bytes, for `list`, the length is in number of elements, and for `map`, the length is in number of key/value element pairs.

For `list` and `map`, sub elements are regular CBOR elements with their own _Header_, _Value_ and _Payload_. `list`s and `map`s can be recursively encoded.
If a `list` or `map` has _Additional info_ 31, it is "indefinite-length", which means it has an "unknown" number of elements.
Instead, its end is marked by a `prim` with _Additional info_ 31 (byte value 0xFF).

Usage Example
=============

Code generation
---------------

This example is is taken from [test3_simple](tests/decode/test3_simple/).

If your CDDL file contains the following code:

```cddl
Timestamp = bstr .size 8

; Comments are denoted with a semicolon
Pet = [
    name: [ +tstr ],
    birthday: Timestamp,
    species: (cat: 1) / (dog: 2) / (other: 3),
]
```
Call the Python script:

```sh
python3 <zcbor base>/zcbor/zcbor.py -c pet.cddl code -d -t Pet --oc pet_decode.c --oh pet_decode.h
# or
zcbor -c pet.cddl code -d -t Pet --oc pet_decode.c --oh pet_decode.h
```

And use the generated code with

```c
#include <pet_decode.h> /* The name of the header file is taken from the name of
                           the cddl file, but can also be specifiec when calling
                           the script. */

/* ... */

/* The following type and function refer to the Pet type in the CDDL, which
 * has been specified as an --entry-types (-t) when invoking zcbor. */
Pet_t pet;
size_t decode_len;
bool success = cbor_decode_Pet(input, sizeof(input), &pet, &decode_len);
```

The process is the same for encoding, except:
 - Change `-d` to `-e` when invoking zcbor
 - Input parameters become output parameters and vice versa in the code:

```c
#include <pet_encode.h> /* The name of the header file is taken from the name of
                           the cddl file, but can also be specifiec when calling
                           the script. */

/* ... */

/* The following type and function refer to the Pet type in the CDDL, which
 * has been specified as an --entry-types (-t) when invoking zcbor. */
Pet_t pet = { /* Initialize with desired data. */ };
uint8_t output[100]; /* 100 is an example. Must be large enough for data to fit. */
size_t out_len;
bool success = cbor_encode_Pet(output, sizeof(output), &pet, &out_len);
```

CBOR decoding/encoding library
------------------------------

For encoding:

```c
#include <zcbor_encode.h>

uint8_t payload[100];
zcbor_state_t state;
zcbor_new_state(&state, 1, payload, sizeof(payload), 0);

res = res && zcbor_list_start_encode(&state, 0);
res = res && zcbor_tstr_put(&state, "first");
res = res && zcbor_tstr_put(&state, "second");
res = res && zcbor_list_end_encode(&state, 0);
uint8_t timestamp[8] = {1, 2, 3, 4, 5, 6, 7, 8};
struct zcbor_string timestamp_str = {
  .value = timestamp,
  .len = sizeof(timestamp),
};
res = res && zcbor_bstr_encode(&state, &timestamp_str);
res = res && zcbor_uint32_put(&state, 2 /* dog */);
res = res && zcbor_list_end_encode(&state, 0);

```

Converting
----------

Here is an example call for converting from YAML to CBOR:

```sh
python3 <zcbor base>/zcbor/zcbor.py -c pet.cddl convert -t Pet -i mypet.yaml -o mypet.cbor
# or
zcbor -c pet.cddl convert -t Pet -i mypet.yaml -o mypet.cbor
```

Which takes a yaml structure from mypet.yaml, validates it against the Pet type in the CDDL description in pet.cddl, and writes binary CBOR data to mypet.cbor.

See the tests in  <zcbor base>/tests/ for examples of using the python module

Running tests
=============

The tests for the generated code are based on Zephyr ztests.
Tests for the conversion functions in the script are implemented with the unittest module.

There are also test.sh scripts to quickly run all tests.
[`tests/test.sh`](tests/test.sh) runs all tests, including python tests in [`tests/scripts`](tests/scripts).

These tests are dependent upon the `pycodestyle` package from `pip`.
Run these scripts with no arguments.

To set up the environment to run the ztest tests, follow [Zephyr's Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html), or see the workflow in the [`.github`](.github) directory.

Command line documentation
==========================

Added via `add_helptext.py`

zcbor --help
------------

```
usage: zcbor [-h] [--version] -c CDDL [--default-max-qty DEFAULT_MAX_QTY]
             [--no-prelude] [-v]
             {code,convert} ...

Parse a CDDL file and validate/convert between YAML, JSON, and CBOR. Can also
generate C code for validation/encoding/decoding of CBOR. Note that the other
top-level arguments (e.g. -c) must appear before {code/convert}. See zcbor
code/convert --help to see their respective specialized arguments.

positional arguments:
  {code,convert}

options:
  -h, --help            show this help message and exit
  --version             show program's version number and exit
  -c CDDL, --cddl CDDL  Path to one or more input CDDL file(s). Passing
                        multiple files is equivalent to concatenating them.
  --default-max-qty DEFAULT_MAX_QTY, --dq DEFAULT_MAX_QTY
                        Default maximum number of repetitions when no maximum
                        is specified. This is needed to construct complete C
                        types. This is relevant for the generated code. It is
                        not relevant for converting, except when handling data
                        that will be decoded by generated code. The default
                        value of this option is 3. Set it to a large number
                        when not relevant. When generating code, the
                        default_max_qty can usually be set to a text symbol,
                        to allow it to be configurable when building the code.
                        This is not always possible, as sometimes the value is
                        needed for internal computations. If so, the script
                        will raise an exception.
  --no-prelude          Exclude the standard CDDL prelude from the build. The
                        prelude can be viewed at zcbor/cddl/prelude.cddl in
                        the repo, or together with the script.
  -v, --verbose         Print more information while parsing CDDL and
                        generating code.

```

zcbor code --help
-----------------

```
usage: zcbor code [-h] [--output-c OUTPUT_C] [--output-h OUTPUT_H]
                  [--output-h-types OUTPUT_H_TYPES] [--copy-sources]
                  [--output-cmake OUTPUT_CMAKE] -t ENTRY_TYPES
                  [ENTRY_TYPES ...] [-d] [-e] [--time-header]
                  [--git-sha-header] [-b {32,64}]
                  [--include-prefix INCLUDE_PREFIX] [-s]

Parse a CDDL file and produce C code that validates and xcodes CBOR.
The output from this script is a C file and a header file. The header file
contains typedefs for all the types specified in the cddl input file, as well
as declarations to xcode functions for the types designated as entry types when
running the script. The c file contains all the code for decoding and validating
the types in the CDDL input file. All types are validated as they are xcoded.

Where a `bstr .cbor <Type>` is specified in the CDDL, AND the Type is an entry
type, the xcoder will not xcode the string, only provide a pointer into the
payload buffer. This is useful to reduce the size of typedefs, or to break up
decoding. Using this mechanism is necessary when the CDDL contains self-
referencing types, since the C type cannot be self referencing.

This script requires 'regex' for lookaround functionality not present in 're'.

options:
  -h, --help            show this help message and exit
  --output-c OUTPUT_C, --oc OUTPUT_C
                        Path to output C file. If both --decode and --encode
                        are specified, _decode and _encode will be appended to
                        the filename when creating the two files. If not
                        specified, the path and name will be based on the
                        --output-cmake file. A 'src' directory will be created
                        next to the cmake file, and the C file will be placed
                        there with the same name (except the file extension)
                        as the cmake file.
  --output-h OUTPUT_H, --oh OUTPUT_H
                        Path to output header file. If both --decode and
                        --encode are specified, _decode and _encode will be
                        appended to the filename when creating the two files.
                        If not specified, the path and name will be based on
                        the --output-cmake file. An 'include' directory will
                        be created next to the cmake file, and the C file will
                        be placed there with the same name (except the file
                        extension) as the cmake file.
  --output-h-types OUTPUT_H_TYPES, --oht OUTPUT_H_TYPES
                        Path to output header file with typedefs (shared
                        between decode and encode). If not specified, the path
                        and name will be taken from the output header file
                        (--output-h), with '_types' added to the file name.
  --copy-sources        Copy the non-generated source files (zcbor_*.c/h) into
                        the same directories as the generated files.
  --output-cmake OUTPUT_CMAKE
                        Path to output CMake file. The filename of the CMake
                        file without '.cmake' is used as the name of the CMake
                        target in the file. The CMake file defines a CMake
                        target with the zcbor source files and the generated
                        file as sources, and the zcbor header files' and
                        generated header files' folders as
                        include_directories. Add it to your project via
                        include() in your CMakeLists.txt file, and link the
                        target to your program. This option works with or
                        without the --copy-sources option.
  -t ENTRY_TYPES [ENTRY_TYPES ...], --entry-types ENTRY_TYPES [ENTRY_TYPES ...]
                        Names of the types which should have their xcode
                        functions exposed.
  -d, --decode          Generate decoding code. Either --decode or --encode or
                        both must be specified.
  -e, --encode          Generate encoding code. Either --decode or --encode or
                        both must be specified.
  --time-header         Put the current time in a comment in the generated
                        files.
  --git-sha-header      Put the current git sha of zcbor in a comment in the
                        generated files.
  -b {32,64}, --default-bit-size {32,64}
                        Default bit size of integers in code. When integers
                        have no explicit bounds, assume they have this bit
                        width. Should follow the bit width of the architecture
                        the code will be running on.
  --include-prefix INCLUDE_PREFIX
                        When #include'ing generated files, add this path
                        prefix to the filename.
  -s, --short-names     Attempt to make most generated struct member names
                        shorter. This might make some names identical which
                        will cause a compile error. If so, tweak the CDDL
                        labels or layout, or disable this option. This might
                        also make enum names different from the corresponding
                        union members.

```

zcbor convert --help
--------------------

```
usage: zcbor convert [-h] -i INPUT [--input-as {yaml,json,cbor,cborhex}]
                     [-o OUTPUT] [--output-as {yaml,json,cbor,cborhex,c_code}]
                     [--c-code-var-name C_CODE_VAR_NAME] -t ENTRY_TYPE

Parse a CDDL file and verify/convert between CBOR and YAML/JSON. The script
decodes the CBOR/YAML/JSON data from a file or stdin and verifies that it
conforms to the CDDL description. The script fails if the data does not
conform. The script can also be used to just verify. JSON and YAML do not
support all data types that CBOR/CDDL supports. bytestrings (BSTR), tags, and
maps with non-text keys need special handling: All strings in JSON/YAML are
text strings. If a BSTR is needed, use a dict with a single entry, with "bstr"
as the key, and the byte string (as a hex string) as the value, e.g. {"bstr":
"0123456789abcdef"}. The value can also be another type, e.g. which will be
interpreted as a BSTR with the given value as contents (in cddl: 'bstr .cbor
SomeType'). E.g. {"bstr": ["first element", 2, [3]]} Dicts in JSON/YAML only
support text strings for keys, so if a dict needs other types of keys,
encapsulate the key and value into a dict (n is an arbitrary integer): e.g.
{"name": "foo", "keyvaln": {"key": 123, "val": "bar"}} which will conform to
the CDDL {tstr => tstr, int => tstr}. Tags are specified by a dict with two
elements, e.g. {"tag": 1234, "value": ["tagged string within list"]}
'undefined' is specified as a list with a single text entry:
"zcbor_undefined".

options:
  -h, --help            show this help message and exit
  -i INPUT, --input INPUT
                        Input data file. The option --input-as specifies how
                        to interpret the contents. Use "-" to indicate stdin.
  --input-as {yaml,json,cbor,cborhex}
                        Which format to interpret the input file as. If
                        omitted, the format is inferred from the file name.
                        .yaml, .yml => YAML, .json => JSON, .cborhex => CBOR
                        as hex string, everything else => CBOR
  -o OUTPUT, --output OUTPUT
                        Output data file. The option --output-as specifies how
                        to interpret the contents. If --output is omitted, no
                        conversion is done, only verification of the input.
                        Use "-" to indicate stdout.
  --output-as {yaml,json,cbor,cborhex,c_code}
                        Which format to interpret the output file as. If
                        omitted, the format is inferred from the file name.
                        .yaml, .yml => YAML, .json => JSON, .c, .h => C code,
                        .cborhex => CBOR as hex string, everything else =>
                        CBOR
  --c-code-var-name C_CODE_VAR_NAME
                        Only relevant together with '--output-as c_code' or .c
                        files.
  -t ENTRY_TYPE, --entry-type ENTRY_TYPE
                        Name of the type (from the CDDL) to interpret the data
                        as.

```
