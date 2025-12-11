#include "rdtsc_test.h"
#include <stdint.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #define ARCH_X86
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ARCH_ARM64
#elif defined(__arm__) || defined(_M_ARM)
    #define ARCH_ARM32
#endif

#ifdef ARCH_X86

uint64_t _RDTSC(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ (
        "rdtsc"
        : "=a"(lo), "=d"(hi)
    );
    return ((uint64_t)hi << 32) | lo;
}

#elif defined(ARCH_ARM64)

uint64_t _RDTSC(void) {
    uint64_t val;
    __asm__ __volatile__ (
        "mrs %0, cntvct_el0"
        : "=r"(val)
    );
    return val;
}

#elif defined(ARCH_ARM32)

uint64_t _RDTSC(void) {
    uint32_t pmccntr;
    uint32_t pmuseren;
    uint32_t pmcntenset;
    
    __asm__ __volatile__ (
        "mrc p15, 0, %0, c9, c14, 0"
        : "=r"(pmuseren)
    );
    if ((pmuseren & 1) == 0) {
        __asm__ __volatile__ (
            "mcr p15, 0, %0, c9, c14, 0"
            :: "r"(1)
        );
    }
    
    __asm__ __volatile__ (
        "mrc p15, 0, %0, c9, c12, 1"
        : "=r"(pmcntenset)
    );
    if ((pmcntenset & 0x80000000) == 0) {
        __asm__ __volatile__ (
            "mcr p15, 0, %0, c9, c12, 1"
            :: "r"(0x80000000)
        );
    }
    
    __asm__ __volatile__ (
        "mrc p15, 0, %0, c9, c13, 0"
        : "=r"(pmccntr)
    );
    return (uint64_t)pmccntr;
}

#else

#include <time.h>

uint64_t _RDTSC(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

#endif

#if defined(ARCH_ARM64)

uint64_t _LMULDWORD(uint32_t low, uint32_t high) {
    uint64_t result;
    __asm__ __volatile__ (
        "umull %0, %w1, %w2"
        : "=r"(result)
        : "r"((uint64_t)low), "r"((uint64_t)high)
    );
    return result;
}

#elif defined(ARCH_ARM32)

uint64_t _LMULDWORD(uint32_t low, uint32_t high) {
    uint64_t result_low, result_high;
    __asm__ __volatile__ (
        "umull %Q0, %R0, %1, %2"
        : "=r"(result_low)
        : "r"(low), "r"(high)
    );
    return result_low;
}

#else

uint64_t _LMULDWORD(uint32_t low, uint32_t high) {
    return (uint64_t)low * (uint64_t)high;
}

#endif

#ifdef ARCH_ARM64

void _EXECMASM(void) {
    register uint64_t x0 __asm__("x0") = 0;
    register uint64_t x1 __asm__("x1") = 10000;
    
    __asm__ __volatile__ (
        "1:\n\t"
        "add %0, %0, %1\n\t"
        "sub %0, %0, %1\n\t"
        "b 2f\n\t"
        "2:\n\t"
        "subs %1, %1, #1\n\t"
        "bne 1b"
        : "+r"(x0), "+r"(x1)
        :
        : "cc"
    );
}

#elif defined(ARCH_ARM32)

void _EXECMASM(void) {
    register uint32_t r0 __asm__("r0") = 0;
    register uint32_t r1 __asm__("r1") = 10000;
    
    __asm__ __volatile__ (
        "1:\n\t"
        "add %0, %0, %1\n\t"
        "sub %0, %0, %1\n\t"
        "b 2f\n\t"
        "2:\n\t"
        "subs %1, %1, #1\n\t"
        "bne 1b"
        : "+r"(r0), "+r"(r1)
        :
        : "cc"
    );
}

#else

void _EXECMASM(void) {
    volatile uint32_t eax = 0;
    volatile uint32_t ecx = 10000;
    
    while (ecx > 0) {
        eax += ecx;
        eax -= ecx;
        ecx--;
    }
}

#endif

#ifdef ARCH_ARM64

void AsmSchedule(void *param) {
    register uint64_t x0 __asm__("x0");
    register uint64_t x1 __asm__("x1");
    register uint64_t x2 __asm__("x2");
    register uint64_t x3 __asm__("x3");
    register uint64_t x4 __asm__("x4");
    register uint64_t x5 __asm__("x5");
    register uint64_t x6 __asm__("x6");
    register uint64_t x7 __asm__("x7");
    register uint64_t x8 __asm__("x8");
    register uint64_t x9 __asm__("x9");
    register uint64_t x10 __asm__("x10");
    register uint64_t x11 __asm__("x11");
    register uint64_t x12 __asm__("x12");
    register uint64_t x13 __asm__("x13");
    register uint64_t x14 __asm__("x14");
    register uint64_t x15 __asm__("x15");
    register uint64_t x29 __asm__("x29");
    register uint64_t x30 __asm__("x30");
    
    __asm__ __volatile__ (
        "stp x29, x30, [sp, #-16]!\n\t"
        "stp x0, x1, [sp, #-16]!\n\t"
        "stp x2, x3, [sp, #-16]!\n\t"
        "stp x4, x5, [sp, #-16]!\n\t"
        "stp x6, x7, [sp, #-16]!\n\t"
        "stp x8, x9, [sp, #-16]!\n\t"
        "stp x10, x11, [sp, #-16]!\n\t"
        "stp x12, x13, [sp, #-16]!\n\t"
        "stp x14, x15, [sp, #-16]!\n\t"
        "mov x1, %0\n\t"
        "mov x0, sp\n\t"
        "bl Schedule\n\t"
        "mov sp, x0\n\t"
        "ldp x14, x15, [sp], #16\n\t"
        "ldp x12, x13, [sp], #16\n\t"
        "ldp x10, x11, [sp], #16\n\t"
        "ldp x8, x9, [sp], #16\n\t"
        "ldp x6, x7, [sp], #16\n\t"
        "ldp x4, x5, [sp], #16\n\t"
        "ldp x2, x3, [sp], #16\n\t"
        "ldp x0, x1, [sp], #16\n\t"
        "ldp x29, x30, [sp], #16"
        :
        : "r"(param)
        : "memory", "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x29", "x30"
    );
}

#elif defined(ARCH_ARM32)

void AsmSchedule(void *param) {
    register uint32_t r0 __asm__("r0");
    register uint32_t r1 __asm__("r1");
    register uint32_t r2 __asm__("r2");
    register uint32_t r3 __asm__("r3");
    register uint32_t r4 __asm__("r4");
    register uint32_t r5 __asm__("r5");
    register uint32_t r6 __asm__("r6");
    register uint32_t r7 __asm__("r7");
    register uint32_t r8 __asm__("r8");
    register uint32_t r9 __asm__("r9");
    register uint32_t r10 __asm__("r10");
    register uint32_t r11 __asm__("r11");
    register uint32_t r12 __asm__("r12");
    register uint32_t lr __asm__("lr");
    
    __asm__ __volatile__ (
        "push {r0-r12, lr}\n\t"
        "mov r1, %0\n\t"
        "mov r0, sp\n\t"
        "bl Schedule\n\t"
        "mov sp, r0\n\t"
        "pop {r0-r12, lr}"
        :
        : "r"(param)
        : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "lr"
    );
}

#else

void AsmSchedule(void *param) {
    void *stack_ptr = __builtin_frame_address(0);
    Schedule(stack_ptr, param);
}

#endif
