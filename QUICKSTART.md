# Quick Start Guide

## Build the Library

```bash
# Using Make
make

# Or using the build script
./build.sh
```

This creates:
- `librdtsctest.a` (static library)
- `librdtsctest.so` (shared library)

## Run the Tests

```bash
# Compile test program
gcc -o test_rdtsc test_rdtsc.c librdtsctest.a

# Run tests
./test_rdtsc
```

## Run the Examples

```bash
# Compile example program
gcc -o example example.c librdtsctest.a

# Run examples
./example
```

## Use in Your Project

### Step 1: Include the header
```c
#include "rdtsc_test.h"
```

### Step 2: Implement Schedule function (if using AsmSchedule)
```c
void Schedule(void *stack_ptr, void *param) {
    // Your implementation here
}
```

### Step 3: Use the functions
```c
// Read cycle counter
uint64_t cycles = _RDTSC();

// Multiply two 32-bit numbers
uint64_t result = _LMULDWORD(1000000, 2000000);

// Run calibration loop
_EXECMASM();

// Call with register preservation
AsmSchedule(my_param);
```

### Step 4: Compile your program
```bash
# Link with static library
gcc -o myapp myapp.c librdtsctest.a

# Or compile directly
gcc -o myapp myapp.c rdtsc_test.c
```

## Cross-Compile for ARM

### ARM64 (aarch64)
```bash
CC=aarch64-linux-gnu-gcc make
```

### ARM32 (armhf)
```bash
CC=arm-linux-gnueabihf-gcc make
```

## Minimal Example

```c
#include <stdio.h>
#include "rdtsc_test.h"

void Schedule(void *stack_ptr, void *param) {
    printf("Schedule called\n");
}

int main(void) {
    uint64_t start = _RDTSC();
    
    // Your code here
    
    uint64_t end = _RDTSC();
    printf("Elapsed: %llu cycles\n", end - start);
    
    return 0;
}
```

Compile and run:
```bash
gcc -o minimal minimal.c rdtsc_test.c
./minimal
```

## Troubleshooting

### "undefined reference to Schedule"
Make sure you implement the `Schedule` function if you use `AsmSchedule`.

### ARM32 cycle counter not working
On some systems, run:
```bash
sudo sh -c 'echo 1 > /proc/sys/kernel/perf_event_paranoid'
```

### Cross-compilation issues
Install cross-compilers:
```bash
# Debian/Ubuntu
sudo apt-get install gcc-aarch64-linux-gnu gcc-arm-linux-gnueabihf
```

## Documentation

- `README.md` - Full documentation
- `PORTING_NOTES.md` - Technical details of x86 to ARM port
- `MIGRATION_SUMMARY.md` - Summary of changes
- `rdtsc_test.h` - API reference (header file)
