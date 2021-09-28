Schema based data manipulation and code generation
==================================================

CDDL is a human-readable data description language defined in [IETF RFC 8610](https://datatracker.ietf.org/doc/rfc8610/).
By invoking `cddl-gen` (when installed via Pip or setup.py), or the Python script [cddl_gen.py](cddl_gen/cddl_gen.py) directly, you can generate C code that validates/encodes/decodes CBOR data conforming to a CDDL schema.
cddl-gen can also validate and convert CBOR data to and from JSON/YAML, either from the command line, or imported as a module.
Finally, the package contains a light-weight CBOR encoding/decoding library in C.
This library is used by the generated code, and can also be used directly in your own code.

Features
========

Here are some possible ways cddl-gen can be used:

 - Python scripting:
   - Validate a YAML file and translate it into CBOR e.g. for transmission.
   - Validate a YAML/JSON/CBOR file before processing it with some other tool
   - Decode and validate incoming CBOR data into human-readable YAML/JSON.
   - As part of a python script that processes YAML/JSON/CBOR files. cddl-gen is compatible with PyYAML and can additionally provide validation and/or easier inspection via named tuples.
 - C code:
   - Generate C code for validating and decoding or encoding CBOR, for use in optimized or constrained environments, such as microcontrollers.
   - Provide a low-footprint CBOR decoding/encoding library similar to TinyCBOR/QCBOR/NanoCBOR.

Python scripting
================

Invoking cddl_gen.py from the command line
------------------------------------------

The cddl_gen.py script can directly read CBOR, YAML, or JSON data and validate it against a CDDL description.
It can also freely convert the data between CBOR/YAML/JSON.
It can also output the data to a C file formatted as a byte array.

The following is a generalized example for converting (and validating) data from the command line.
The script infers the data format from the file extension, but the format can also be specified explicitly.
See `cddl-gen convert --help` for more information.

```sh
python3 <cddl-gen base>/cddl_gen/cddl_gen.py -c <CDDL description file> convert -t <which CDDL type to expect> -i <input data file> -o <output data file>
```

Or invoke its command line executable (if installed via `pip`):

```sh
cddl-gen -c <CDDL description file> convert -t <which CDDL type to expect> -i <input data file> -o <output data file>
```

Note that since CBOR supports more data types than YAML and JSON, cddl-gen uses an idiomatic format when converting to/from YAML/JSON.
This is relevant when handling YAML/JSON conversions of data that uses the unsupported features.
The following data types are supported by CBOR, but not by YAML (and JSON which is a subset of YAML):

 1. bytestrings: YAML supports only text strings. In YAML, bytestrings ('<bytestring>') are represented as {"bstr": "<hex-formatted bytestring>"}, or as {"bstr": <any type>} if the CBOR bytestring contains CBOR-formatted data, in which the data is decoded into <any type>.
 2. map keys other than text string: In YAML, such key value pairs are represented as {"keyval<unique int>": {"key": <key, not text>, "val": <value>}}
 3. tags: In cbor2, tags are represented by a special type, cbor2.CBORTag. In YAML, these are represented as {"tag": <tag number>, "val": <tagged data>}.

Importing cddl_gen in a Python script
-------------------------------------

Importing cddl_gen gives access to the DataTranslator class which is used to implement the command line conversion features.
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

When calling cddl-gen with the argument `--output-cmake <file path>`, a cmake file will be created at that location.
The cmake file creates a cmake target and adds the generated and non-generated source files, and the include directories to the header files.
This cmake file can then be included in your project's `CMakeLists.txt` file, and the target can be linked into your project.
This is demonstrated in the tests, e.g. at tests/cbor_decode/test3_simple/CMakeLists.txt.
cddl-gen can be instructed to copy the non-generated sources to the same location as the generated sources with `--copy-sources`.

CBOR decoding/encoding library
==============================

The CBOR library found at [headers](include) and [source](src) is used by the generated code, but can also be used directly.
If so, you must instantiate a `cbor_state_t` object as well as a `cbor_state_backups_t` object (backups can be NULL in simple use cases).

The elem_count member refers to the number of encoded objects in the current list or map.
elem_count starts again when entering a nested list or map, and is restored when exiting.

elem_count is one reason for needing "backup" states (the other is to allow rollback of the payload).
You need a number of backups corresponding to the maximum number of nested levels in your data.

Backups are needed for encoding if you are using canonical encoding (CDDL_CBOR_CANONICAL), or using the bstrx_cbor_* functions.
Backups are needed for decoding if there are any lists, maps, or CBOR-encoded strings in the data.

Note that the benefits of using the library directly is greater for encoding than for decoding.
For decoding, the code generation will provide a number of checks that are tedious to write manually, and easy to forget.

```c
/** The number of states must be at least equal to one more than the maximum
 *  nested depth of the data.
 */
cbor_state_t states[n];

/** Initialize the states. After calling this, states[0] is ready to be used
 *  with the encoding/decoding APIs.
 *  elem_count must be the maximum expected number of top-level elements when
 *  decoding (1 if the data is wrapped in a list).
 *  When encoding, elem_count must be 0.
 */
new_state(states, n, payload, payload_len, elem_count);
```

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

Note that in the cddl_gen script and its generated code, the number of entries supported via `*` and `+` is affected by the default_max_qty value.

Any element can be labeled with `:`.
The label is only for readability and does not impact the data structure in any way.
E.g. `Foo = [name: tstr, age: uint]` is equivalent to `Foo = [tstr, uint]`.

See [test3_simple](tests/cbor_decode/test3_simple/) for CDDL example code.

Usage Example
=============

Code generation
---------------

This example is is taken from [test3_simple](tests/cbor_decode/test3_simple/).

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
python3 <cddl-gen base>/cddl_gen/cddl_gen.py -c pet.cddl code -d -t Pet --oc pet_decode.c --oh pet_decode.h
# or
cddl-gen -c pet.cddl code -d -t Pet --oc pet_decode.c --oh pet_decode.h
```

And use the generated code with

```c
#include <pet_decode.h> /* The name of the header file is taken from the name of
                           the cddl file, but can also be specifiec when calling
                           the script. */

/* ... */

/* The following type and function refer to the Pet type in the CDDL, which
 * has been specified as an --entry-types (-t) when invoking cddl-gen. */
Pet_t pet;
uint32_t decode_len;
bool success = cbor_decode_Pet(input, sizeof(input), &pet, &decode_len);
```

The process is the same for encoding, except:
 - Change `-d` to `-e` when invoking cddl-gen
 - Input parameters become output parameters and vice versa in the code:

```c
#include <pet_encode.h> /* The name of the header file is taken from the name of
                           the cddl file, but can also be specifiec when calling
                           the script. */

/* ... */

/* The following type and function refer to the Pet type in the CDDL, which
 * has been specified as an --entry-types (-t) when invoking cddl-gen. */
Pet_t pet = { /* Initialize with desired data. */ };
uint8_t output[100]; /* 100 is an example. Must be large enough for data to fit. */
uint32_t out_len;
bool success = cbor_encode_Pet(output, sizeof(output), &pet, &out_len);
```

CBOR decoding/encoding library
------------------------------

For encoding:

```c
#include <cbor_encode.h>

uint8_t payload[100];
cbor_state_t state;
new_state(&state, 1, payload, sizeof(payload), 0);

res = res && list_start_encode(&state, 0);
res = res && tstrx_put(&state, "first");
res = res && tstrx_put(&state, "second");
res = res && list_end_encode(&state, 0);
uint8_t timestamp[8] = {1, 2, 3, 4, 5, 6, 7, 8};
cbor_string_type_t timestamp_str = {
  .value = timestamp,
  .len = sizeof(timestamp),
};
res = res && bstrx_encode(&state, &timestamp_str);
res = res && uintx32_put(&state, 2 /* dog */);
res = res && list_end_encode(&state, 0);

```

Converting
----------

Here is an example call for converting from YAML to CBOR:

```sh
python3 <cddl-gen base>/cddl_gen/cddl_gen.py -c pet.cddl convert -t Pet -i mypet.yaml -o mypet.cbor
# or
cddl-gen -c pet.cddl convert -t Pet -i mypet.yaml -o mypet.cbor
```

Which takes a yaml structure from mypet.yaml, validates it against the Pet type in the CDDL description in pet.cddl, and writes binary CBOR data to mypet.cbor.

See the tests in  <cddl-gen base>/tests/ for examples of using the python module

Running tests
=============

The tests for the generated code are based on Zephyr ztests.
Tests for the conversion functions in the script are implemented with the unittest module.

There are also test.sh scripts to quickly run all tests.
[`tests/test.sh`](tests/test.sh) runs all tests, including python tests in [`tests/scripts`](tests/scripts).
[`tests/cbor_decode/test.sh`](tests/cbor_decode/test.sh) runs all decoding tests.
[`tests/cbor_encode/test.sh`](tests/cbor_encode/test.sh) runs all encoding tests.

These tests are dependent upon the `pycodestyle` package from `pip`.
Run these scripts with no arguments.

To set up the environment to run the ztest tests, follow [Zephyr's Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html), or see the workflow in the [`.github`](.github) directory.
