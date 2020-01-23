Generate code from CDDL description
===================================

CDDL is a human-readable description language defined in [IETF RFC 8610](https://datatracker.ietf.org/doc/rfc8610/).
By calling the Python script [cddl_gen.py](scripts/cddl_gen.py), you can generate C code that validates and decodes CBOR data against a CDDL schema.

The generated code depends on a CBOR decoding library ([cbor_decode](include/cbor_decode.h)).
There are tests for the code generation in [tests/](tests/cbor_decode/).
For now the tests require [Zephyr](https://github.com/zephyrproject-rtos/zephyr) (if your shell is set up to build Zephyr samples, the tests should also build).

Features
========

The generated code consists of:
 - A header file containing typedefs for the types defined in the CDDL, as well as declarations for decoding functions for some types (those specified as entry types).
 - A C file containing all the decoding code.
   The code is split across multiple functions, and each function contains a single `if` statement which "and"s and "or"s together calls into the cbor_decode library or to other generated decoding functions.

CDDL allows placing restrictions on the members of your data structure.
Restictions can be on type, on content (e.g. values/sizes of ints or strings), and repetition (e.g. the number of members in a list).
The generated code will validate the input, which means that it will check all the restriction set in the CDDL description, and fail if a restriction is broken.

The cbor_decode library does most of the actual extraction and validation of individual values.
Currently, only decoding of CBOR is supported.

Build system
------------

There is some CMake code available which requires Zephyr to run.
When you call the [`target_cddl_source()`](cmake/extensions.cmake) CMake function, it sets up build steps necessary to call the script on the provided CDDL file, and adds the generated file as well as the cbor_decode library to your project.
As long as the `target_cddl_source()` function is called in your project, you should be able to #include the generated file and use it in your code.

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
 - `x**y`: Between x and y times, inclusive. E.g. `Foo = {4**8(int => bstr)}` where Foo is a map with 4 to 8 key/value pairs where each key is an int and each value is a bstr.

Any element can be labeled with `:`.
The label is only for readability and does not impact the data structure in any way.
E.g. `Foo = [name: tstr, age: uint]` is equivalent to `Foo = [tstr, uint]`.

See [test3_simple](tests/cbor_decode/test3_simple/) for CDDL example code.

Usage Example
=============

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
Call the Python script

```sh
python3 <ccdlgen base>/scripts/cddl_gen.py -i pet.cddl -t Pet
```

Or add the following line to your CMake code:

```cmake
target_cddl_source(app pet.cddl ENTRY_TYPES Pet)
```

And use the generated code with

```c
#include <pet.h> /* The name of the header file is taken from the name of the
                    cddl file. */

/* ... */

/* The following type and function refer to the Pet type in the CDDL, which
 * has been specified as an ENTRY_TYPE in the cmake call. */
Pet_t pet;
bool success = cbor_decode_Pet(input, sizeof(input), &pet);
```
