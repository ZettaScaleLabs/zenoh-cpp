<img src="https://raw.githubusercontent.com/eclipse-zenoh/zenoh/master/zenoh-dragon.png" height="150">

# Eclipse Zenoh C++ API - QNX Support

**Support for QNX 7.1 is currently a work in progress with limited functionality supported.**

**Zenoh-Pico is not currently supported.**

To use the Zenoh C++ API for QNX 7.1 a QNX development environment is required as well as a build of [this branch of the Zenoh C API](https://github.com/ZettaScaleLabs/zenoh-c/tree/qnx-port-1.0.0).

## Installing the Zenoh C++ API for QNX 7.1

The Zenoh C++ API is installed as per the instructions in the [README](README.md#how-to-build-and-install-it).

## Building the Zenoh C++ API Examples for QNX 7.1

It is expected that you have a QNX compatible version of zenoh-c installed.

To build examples run:

```
CC=qcc CXX=qcc CFLAGS=-Vgcc_ntox86_64_cxx AR=ntox86_64-ar cmake ..
cmake --build . --target examples
```

Examples are placed into build/examples/zenohc directory.
