
# Simple 'hello world' sample

This sample encodes and then decodes the string "Hello World", and prints the decoded string.
It shows how to instantiate zcbor state variables, and how to use them with the encoding and decoding API.
This sample does not use the zcbor script tool.

### To build:

```
mkdir build
cmake . -Bbuild
make -C build
```

### To run:

```
build/app
```

### Expected output:

> Decoded string: 'Hello World'
