zcbor
=====

After installing, zcbor can be invoked on the command line as `zcbor`, or included as a module via `import zcbor`.

This package has 2 uses:

1. Schema-based validation and conversion of YAML/JSON/CBOR data.
2. Generating C code that decodes/encodes and validates CBOR data according to a schema.

The schema language used by zcbor is [CDDL](https://datatracker.ietf.org/doc/rfc8610/) which allows creating very advanced and detailed schemas.

The PyPi package comes with a C library which is needed by the generated code.
This library is not run by the zcbor package, but in the user's project that includes zcbor-generated code.
If zcbor is asked to generate a cmake file, the file will reference the C library, and if asked to copy sources, zcbor will copy these library files to the given location.

The C library also functions as a standalone CBOR C library, and is used in the [Zephyr RTOS](https://zephyrproject.org/).

Please visit the [Github repository](https://github.com/NordicSemiconductor/zcbor) for more information about this Python package, and the C library.
