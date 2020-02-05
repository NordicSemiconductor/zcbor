Generate code from CDDL description
===================================

CDDL is a human-readable description language defined in an IETF RFC (https://tools.ietf.org/html/rfc8610).
By calling the python script, you can autogenerate C code that validates and decodes CBOR encoded instances of the structure described in a given .cddl file.

The code generation is performed by the Python script cddl_gen.py in scripts/.
The generated code depends on a CBOR decoding library (cbor_decode) in src/.
There are tests for the code generation in tests/.
The tests require Zephyr (If your shell is set up to build Zephyr samples, the tests should also build)

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

There is some Cmake code available which requires Zephyr to run.
The `target_cddl_source()` Cmake function sets up build steps necessary to call the script on the provided CDDL file, and adds the generated file as well as the cbor_decode library to your project.
As long as the `target_cddl_source()` function is called in your project, you should be able to #include the generated file and use it in your code.

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

Or add the following line to your Cmake code:

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
