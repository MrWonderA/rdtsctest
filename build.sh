#!/bin/bash

set -e

echo "Building rdtsctest for $(uname -m) architecture..."

ARCH=$(uname -m)
CC=${CC:-gcc}

case "$ARCH" in
    x86_64|i386|i686)
        echo "Detected x86 architecture"
        ARCH_FLAGS="-march=native"
        ;;
    aarch64|arm64)
        echo "Detected ARM64 architecture"
        ARCH_FLAGS="-march=armv8-a"
        ;;
    armv7l|armv7|arm)
        echo "Detected ARM32 architecture"
        ARCH_FLAGS="-march=armv7-a"
        ;;
    *)
        echo "Unknown architecture: $ARCH"
        echo "Building with default flags..."
        ARCH_FLAGS=""
        ;;
esac

CFLAGS="-Wall -Wextra -O2 ${ARCH_FLAGS}"

echo "Compiler: $CC"
echo "Flags: $CFLAGS"

echo "Compiling rdtsc_test.c..."
$CC $CFLAGS -fPIC -c rdtsc_test.c -o rdtsc_test.o

echo "Creating static library..."
ar rcs librdtsctest.a rdtsc_test.o

echo "Creating shared library..."
$CC -shared -o librdtsctest.so rdtsc_test.o

echo "Build complete!"
echo "  Static library: librdtsctest.a"
echo "  Shared library: librdtsctest.so"
