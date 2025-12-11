# Porting Notes: x86 to ARM Migration

This document describes the technical details of porting rdtsctest from x86 assembly to ARM-compatible C/assembly code.

## Architecture Overview

### Original x86 Implementation
The original implementation was pure 32-bit x86 assembly (GAS syntax) in `rdtsc_test.S`:
- Single assembly file with 4 exported functions
- Used x86-specific instructions (rdtsc, mull, loop)
- Followed x86 calling conventions (cdecl)
- Direct register manipulation (eax, ebx, ecx, edx, etc.)

### New Multi-Architecture Implementation
The new implementation uses C with inline assembly:
- Primary implementation in C for portability
- Architecture detection via preprocessor macros
- Inline assembly for performance-critical operations
- Supports x86 (32/64-bit), ARM (32/64-bit), and fallback implementations

## Function-by-Function Porting Details

### 1. _RDTSC (Cycle Counter)

#### x86 Implementation
```asm
.byte 0x0f
.byte 0x31    # rdtsc instruction
ret           # Returns EDX:EAX (high:low 32-bits)
```

#### ARM64 Implementation
```c
__asm__ __volatile__ (
    "mrs %0, cntvct_el0"  # Read Virtual Count register
    : "=r"(val)
);
```
- Uses `cntvct_el0`: Virtual counter, increments at fixed frequency
- More portable than performance counters (no special permissions needed)
- Returns full 64-bit value directly

#### ARM32 Implementation
```c
# Enable user-mode access to performance counters
"mrc p15, 0, %0, c9, c14, 0"  # Read PMUSERENR
"mcr p15, 0, %0, c9, c14, 0"  # Enable if needed

# Enable cycle counter
"mrc p15, 0, %0, c9, c12, 1"  # Read PMCNTENSET
"mcr p15, 0, %0, c9, c12, 1"  # Enable bit 31

# Read cycle counter
"mrc p15, 0, %0, c9, c13, 0"  # Read PMCCNTR
```
- Uses Performance Monitor Cycle Counter (PMCCNTR)
- May require kernel configuration for user-mode access
- Returns 32-bit value (extended to 64-bit)

#### Fallback Implementation
```c
struct timespec ts;
clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
```
- Uses POSIX clock for maximum portability
- Returns nanoseconds instead of cycles
- Lower precision but works on any platform

### 2. _LMULDWORD (64-bit Multiplication)

#### x86 Implementation
```asm
movl  12(%esp),%eax   # Load first operand
mull  16(%esp)        # Multiply: EDX:EAX = EAX * operand
movl  %eax,%ecx
movl  8(%esp),%eax    # Load second operand
mull  16(%esp)        # Multiply again
addl  %ecx,%edx       # Combine results
ret                   # Returns EDX:EAX
```

#### ARM64 Implementation
```c
__asm__ __volatile__ (
    "umull %0, %w1, %w2"  # Unsigned multiply long
    : "=r"(result)
    : "r"((uint64_t)low), "r"((uint64_t)high)
);
```
- `umull`: Multiplies two 32-bit values, produces 64-bit result
- Single instruction, very efficient

#### ARM32 Implementation
```c
__asm__ __volatile__ (
    "umull %Q0, %R0, %1, %2"  # Unsigned multiply long
    : "=r"(result_low)
    : "r"(low), "r"(high)
);
```
- `umull`: Same as ARM64, but with 32-bit registers
- `%Q0` = low 32 bits, `%R0` = high 32 bits

#### Fallback Implementation
```c
return (uint64_t)low * (uint64_t)high;
```
- Compiler generates optimal code for target architecture
- Works correctly on all platforms

### 3. _EXECMASM (Calibration Loop)

#### x86 Implementation
```asm
movl  $10000,%ecx     # Counter = 10000
l1:
    addl  %ecx,%eax   # Add counter to eax
    subl  %ecx,%eax   # Subtract counter from eax
    jmp   l2
l2:
    loop  l1          # Decrement ecx and loop if not zero
ret
```

#### ARM64 Implementation
```c
"1:\n\t"
"add %0, %0, %1\n\t"    # Add counter to x0
"sub %0, %0, %1\n\t"    # Subtract counter from x0
"b 2f\n\t"              # Branch to label 2
"2:\n\t"
"subs %1, %1, #1\n\t"   # Decrement counter, set flags
"bne 1b"                # Branch if not equal to zero
```
- Direct translation of x86 logic to ARM64 instructions
- Uses condition flags for loop control

#### ARM32 Implementation
```c
"1:\n\t"
"add %0, %0, %1\n\t"    # Add counter to r0
"sub %0, %0, %1\n\t"    # Subtract counter from r0
"b 2f\n\t"              # Branch to label 2
"2:\n\t"
"subs %1, %1, #1\n\t"   # Decrement counter, set flags
"bne 1b"                # Branch if not equal to zero
```
- Same logic as ARM64, using 32-bit registers

#### Fallback Implementation
```c
volatile uint32_t eax = 0;
volatile uint32_t ecx = 10000;
while (ecx > 0) {
    eax += ecx;
    eax -= ecx;
    ecx--;
}
```
- C implementation preserves semantics
- `volatile` prevents compiler optimization

### 4. AsmSchedule (Register Preservation)

#### x86 Implementation
```asm
pushl %eax            # Save all GPRs
pushl %ebx
pushl %ecx
pushl %edx
pushl %esi
pushl %edi
pushl %ebp

movl  0x20(%esp),%eax  # Get parameter from stack
movl  %esp,%ecx        # Get current stack pointer
pushl %eax             # Push parameter
pushl %ecx             # Push stack pointer
call  Schedule         # Call external function
movl  %eax,%esp        # Restore stack pointer from return value

popl  %ebp             # Restore all GPRs
popl  %edi
popl  %esi
popl  %edx
popl  %ecx
popl  %ebx
popl  %eax
ret
```

#### ARM64 Implementation
```c
"stp x29, x30, [sp, #-16]!\n\t"  # Save registers in pairs
"stp x0, x1, [sp, #-16]!\n\t"
# ... (save x2-x15)
"mov x1, %0\n\t"         # x1 = param
"mov x0, sp\n\t"         # x0 = stack pointer
"bl Schedule\n\t"        # Branch and link to Schedule
"mov sp, x0\n\t"         # Restore stack from return value
"ldp x14, x15, [sp], #16\n\t"  # Restore registers
# ... (restore x0-x13, x29, x30)
```
- `stp`/`ldp`: Store/load pair (efficient for saving multiple registers)
- Follows ARM Procedure Call Standard (AAPCS)
- Preserves x0-x15, x29 (frame pointer), x30 (link register)

#### ARM32 Implementation
```c
"push {r0-r12, lr}\n\t"   # Save all registers
"mov r1, %0\n\t"          # r1 = param
"mov r0, sp\n\t"          # r0 = stack pointer
"bl Schedule\n\t"         # Branch and link to Schedule
"mov sp, r0\n\t"          # Restore stack from return value
"pop {r0-r12, lr}"        # Restore all registers
```
- `push`/`pop`: Save/restore multiple registers
- Preserves r0-r12 and lr (link register)

## Calling Convention Differences

### x86 (cdecl)
- Arguments: Pushed on stack (right to left)
- Return value: EDX:EAX (64-bit), EAX (32-bit)
- Caller-saved: EAX, ECX, EDX
- Callee-saved: EBX, ESI, EDI, EBP, ESP

### ARM64 (AAPCS64)
- Arguments: x0-x7 for first 8 arguments
- Return value: x0 (64-bit or less)
- Caller-saved: x0-x18
- Callee-saved: x19-x29
- Special: x29 (FP), x30 (LR), sp

### ARM32 (AAPCS)
- Arguments: r0-r3 for first 4 arguments
- Return value: r0 (32-bit), r0-r1 (64-bit)
- Caller-saved: r0-r3, r12
- Callee-saved: r4-r11
- Special: r13 (SP), r14 (LR), r15 (PC)

## Performance Considerations

### Cycle Counting Accuracy
- **x86 rdtsc**: Very high precision, true cycle count
- **ARM64 cntvct_el0**: Fixed frequency timer, not true cycles but consistent
- **ARM32 pmccntr**: True cycle count when available, may need permissions
- **Fallback clock_gettime**: Nanosecond precision, sufficient for most use cases

### Multiplication Performance
- All architectures have efficient 64-bit multiply instructions
- Compiler optimization handles fallback case well
- No significant performance difference in practice

### Loop Performance
- Assembly versions ensure predictable instruction sequence
- C fallback may be optimized differently by compiler
- Use `volatile` to prevent unwanted optimization

## Build System Changes

### Original
- No build system (manual assembly)
- Single target: x86 32-bit

### New
- Makefile with architecture detection
- Build script (`build.sh`) for convenience
- Supports: static library (.a), shared library (.so)
- Cross-compilation support for ARM targets

## Testing and Validation

To verify correctness on ARM hardware:

1. **Cycle Counter**: Verify monotonic increasing values
2. **Multiplication**: Compare with expected results
3. **Calibration Loop**: Should take consistent time
4. **Register Preservation**: Verify no corruption after Schedule call

See `test_rdtsc.c` for a comprehensive test suite.

## Migration Path for Existing Code

If you have existing code using the x86 assembly version:

1. Replace `#include` or assembly linkage with `#include "rdtsc_test.h"`
2. Recompile with new C implementation
3. Link with `librdtsctest.a` or `librdtsctest.so`
4. No source code changes required (same function signatures)

Example:
```bash
# Old (x86 assembly)
as --32 rdtsc_test.S -o rdtsc_test.o
gcc -m32 myapp.c rdtsc_test.o -o myapp

# New (multi-architecture C)
make
gcc myapp.c -L. -lrdtsctest -o myapp
```

## Known Limitations

1. **ARM32 Cycle Counter**: May require kernel configuration or root privileges
2. **Cycle Frequency**: ARM counters may run at different frequencies than x86
3. **Context Switches**: Performance counters may be affected by OS scheduler
4. **Fallback Mode**: Lower precision when using clock_gettime

## Future Enhancements

Potential improvements:
- Add CPU frequency detection for accurate timing conversion
- Implement RISC-V support
- Add Windows support (QueryPerformanceCounter)
- Provide CMake build system option
- Add ARM NEON optimizations for specific operations
