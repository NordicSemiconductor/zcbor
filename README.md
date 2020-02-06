Generate code from CDDL description
===================================

CDDL is a human-readable description language defined in an IETF RFC (https://tools.ietf.org/html/rfc8610).
By calling the python script, you can autogenerate C code that validates and decodes CBOR encoded instances of the structure described in a given .cddl file.

The code generation is performed by the Python script cddl_gen.py in scripts/.
The generated code depends on a CBOR decoding library (cbor_decode) in src/.
There are tests for the code generation in tests/.
For now the tests require Zephyr (if your shell is set up to build Zephyr samples, the tests should also build).

Features
========

The generated code consists of:
 - A header file containing typedefs corresponding to the types described in the CDDL, as well as functions declarations for decoding functions requested to be exposed (ENTRY_TYPES).
 - A C file containing all the decoding code.
   The decoding code is divided into functions (internal unless the type is an ENTRY_TYPE), and each function generally contains a single `if` statement which "and"s and "or"s together calls into the cbor_decode library or to other generated decoding functions.

CDDL allows specifying data structures both loosely and strictly, both in terms of content (e.g. values of ints or strings), and hierarchy (e.g. the type of an entry, the number of members in a list).
The generated code will validate the input, which means that it will check all the restriction set in the CDDL description, and fail if a restriction is broken.
This is especially important for security applications, but also useful to avoid segfaults and other errors as a result of malformed input.

The cbor_decode library does most of the actual extraction and validation of individual values.
Currently, only decoding of CBOR is supported.

Build system
------------

There is some CMake code available which requires Zephyr to run.
The `target_cddl_source()` CMake function sets up build steps necessary to call the script on the provided CDDL file, and adds the generated file as well as the cbor_decode library to your project.
As long as the `target_cddl_source()` function is called in your project, you should be able to #include the generated file and use it in your code.

Introduction to CDDL
====================

In CDDL you define types from other types.
Types are declared with '=', e.g. "Foo = int" which declares the type Foo which is just an integer, analogous to `typedef int Foo;` in C.
CDDL has the following base types:

 - int: Positive or negative integer
 - uint: Positive integer
 - bstr: Byte string
 - tstr: Text string
 - bool: Boolean
 - nil: Nil/Null value
 - float: Floating point value
 - any: Any single element

and aggregate types:

 - []: List. Elements don't need to have the same type.
 - {}: Map. Keys are declared with '=>' or ':'. Note that ':' is also used for labels.
 - (): Groups. Grouping with no enclosing type, which means that e.g. `Foo = [(int, bstr)]` is equivalent to `Foo = [int, bstr]`.
 - /: Unions. Analogous to unions in C. E.g. `Foo = int/bstr/nil` where Foo is either an int, a bstr, or nil.

Literals can be used instead of the base type names:

 - Number: `Foo = 3`, where Foo is a uint with the additional requirement that it must have the value 3.
 - Number range: `Foo = -100..100`, where Foo is an int with value between -100 and 100.
 - Text string: `Foo = "hello"`, where Foo is a tstr with the requirement that it must be "hello".
 - True/False: `Foo = false`, where Foo is a bool which is always false.

Base types can also be restricted in other ways:

 - .size: Works for integers and strings. E.g. `Foo = uint .size 4` where Foo is a uint exactly 4 bytes long.
 - .cbor/.cborseq: E.g. `Foo = bstr .cbor Bar` where Foo is a bstr whose contents must be CBOR data decodeable as the Bar type (Bar must be defined in the CDDL document)

An element can be repeated:

 - ?: 0 or 1 time. E.g. `Foo = [int, ?bstr]`, where Foo is a list with an int possibly followed by a bstr.
 - *: 0 or more times. E.g. `Foo = [*tstr]`, where Foo is a list containing 0 or more tstrs.
 - +: 1 or more times. E.g. `Foo = [+tstr]`.
 - x**y: Between x and y times, inclusive. E.g. `Foo = {4**8(int => bstr)}` where Foo is a map with 4 to 8 key/value pairs where each key is an int and each value is a bstr.

Any element can be labeled with ':'.
The label is only for readability and does not impact the data structure in any way.
E.g. `Foo = [name: tstr, age: uint]` is equivalent to `Foo = [tstr, uint]`.

CDDL is defined in [IETF RFC 8610](https://datatracker.ietf.org/doc/rfc8610/).

There is a lot of example CDDL code in the tests.
The simplest examples can be found in test3_simple.

Usage Example
=============

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
