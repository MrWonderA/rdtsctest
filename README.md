# rdtsctest

A portable timing and utility library for x86 and ARM architectures, providing high-precision cycle counting and helper functions.

## Overview

This library provides four core functions originally implemented in x86 assembly, now ported to support both x86 and ARM architectures (32-bit and 64-bit) through C with inline assembly optimizations.

## Functions

### 1. `uint64_t _RDTSC(void)`
Reads the CPU cycle counter for high-precision timing measurements.

**Implementation Details:**
- **x86/x86_64**: Uses the `rdtsc` instruction to read the Time Stamp Counter
- **ARM64 (aarch64)**: Uses `cntvct_el0` (Virtual Count register) for cycle counting
- **ARM32**: Uses `pmccntr` (Performance Monitor Cycle Counter)
- **Fallback**: Uses `clock_gettime(CLOCK_MONOTONIC_RAW)` for portability

**Returns:** 64-bit cycle count or timestamp

### 2. `uint64_t _LMULDWORD(uint32_t low, uint32_t high)`
Performs 64-bit multiplication of two 32-bit unsigned integers.

**Implementation Details:**
- **x86**: Uses standard C multiplication with compiler optimization
- **ARM64**: Uses `umull` instruction for optimal performance
- **ARM32**: Uses `umull` instruction
- **Fallback**: Standard C multiplication

**Parameters:**
- `low`: First 32-bit operand
- `high`: Second 32-bit operand

**Returns:** 64-bit product

### 3. `void _EXECMASM(void)`
Executes a deterministic calibration loop (10,000 iterations of add/sub operations).

Used for timing calibration and benchmarking purposes.

**Implementation Details:**
- **ARM64**: Assembly loop using ARM64 instructions
- **ARM32**: Assembly loop using ARM32 instructions
- **Fallback**: C loop implementation

### 4. `void AsmSchedule(void *param)`
Register-preserving wrapper that calls an external `Schedule` function.

Saves all general-purpose registers before calling `Schedule(stack_ptr, param)` and restores them afterward.

**Implementation Details:**
- **ARM64**: Saves/restores x0-x15, x29, x30
- **ARM32**: Saves/restores r0-r12, lr
- **x86**: Compiler-managed register preservation

**Parameters:**
- `param`: Pointer passed to the Schedule function

**External Dependency:**
You must provide an implementation of:
```c
void Schedule(void *stack_ptr, void *param);
```

## Building

### Using Make
```bash
make
```

This will build both static (`librdtsctest.a`) and shared (`librdtsctest.so`) libraries.

### Using Build Script
```bash
./build.sh
```

### Cross-Compilation for ARM

For ARM64:
```bash
CC=aarch64-linux-gnu-gcc make
```

For ARM32:
```bash
CC=arm-linux-gnueabihf-gcc make
```

### Manual Compilation
```bash
# Static library
gcc -Wall -Wextra -O2 -fPIC -c rdtsc_test.c -o rdtsc_test.o
ar rcs librdtsctest.a rdtsc_test.o

# Shared library
gcc -shared -o librdtsctest.so rdtsc_test.o
```

## Usage

### In C/C++ Code

Include the header:
```c
#include "rdtsc_test.h"
```

Link with the library:
```bash
gcc -o myapp myapp.c -L. -lrdtsctest
```

### Example
```c
#include <stdio.h>
#include "rdtsc_test.h"

void Schedule(void *stack_ptr, void *param) {
    // Your scheduling implementation
}

int main(void) {
    // High-precision timing
    uint64_t start = _RDTSC();
    // ... code to measure ...
    uint64_t end = _RDTSC();
    printf("Elapsed: %llu cycles\n", end - start);
    
    // 64-bit multiplication
    uint64_t result = _LMULDWORD(0x12345678, 0xABCDEF01);
    
    // Calibration loop
    _EXECMASM();
    
    // Call with register preservation
    int param = 42;
    AsmSchedule(&param);
    
    return 0;
}
```

## Testing

Build and run the test program:
```bash
gcc -o test_rdtsc test_rdtsc.c rdtsc_test.c
./test_rdtsc
```

## Architecture Support

- **x86**: 32-bit and 64-bit (using original rdtsc instruction)
- **ARM64** (aarch64): Full support with optimized inline assembly
- **ARM32** (armv7, armv7l): Full support with optimized inline assembly
- **Other**: Fallback C implementations for maximum portability

## ARM-Specific Notes

### ARM64 Cycle Counter
The implementation uses `cntvct_el0` which provides a virtual counter that increments at a fixed frequency (typically the system counter frequency). This is more portable than performance counters which may require special permissions.

### ARM32 Cycle Counter
Uses the Performance Monitor Cycle Counter (`pmccntr`). The implementation attempts to enable user-mode access if needed. On some systems, you may need root privileges or specific kernel configurations to access performance counters:
```bash
# May be required on some systems
echo 1 > /proc/sys/kernel/perf_event_paranoid
```

### Register Preservation
The `AsmSchedule` function carefully preserves all general-purpose registers according to the ARM Procedure Call Standard (AAPCS):
- **ARM64**: x0-x15, x29 (frame pointer), x30 (link register)
- **ARM32**: r0-r12, lr (link register)

## License

See original project license.
