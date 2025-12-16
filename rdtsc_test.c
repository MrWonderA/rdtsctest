#include <stdint.h>

#if defined(__i386__) && !defined(RDTSC_TEST_FORCE_C)
/* 32-bit x86 默认使用 rdtsc_test.S 的汇编实现。 */
#else

typedef long long DLONG;

extern uintptr_t Schedule(uintptr_t sp, int queue_id);

DLONG _RDTSC(void)
{
#if defined(__aarch64__)
    uint64_t v;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(v));
    return (DLONG)v;
#elif defined(__arm__)
    uint32_t v;
    __asm__ volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(v));
    return (DLONG)(uint64_t)v;
#elif defined(__i386__) || defined(__x86_64__)
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return (DLONG)(((uint64_t)hi << 32) | lo);
#else
    return 0;
#endif
}

DLONG _LMULDWORD(DLONG value, int mul)
{
    uint64_t a = (uint64_t)(unsigned long long)value;
    uint32_t b = (uint32_t)mul;

    uint32_t a_lo = (uint32_t)a;
    uint32_t a_hi = (uint32_t)(a >> 32);

    uint64_t lo = (uint64_t)a_lo * (uint64_t)b;
    uint64_t hi = (uint64_t)a_hi * (uint64_t)b;

    uint64_t result = lo + ((hi & 0xffffffffULL) << 32);
    return (DLONG)result;
}

int _EXECMASM(void)
{
    volatile uint32_t eax = 0;

    for (volatile uint32_t ecx = 10000; ecx != 0; --ecx) {
        eax += ecx;
        eax -= ecx;
    }

    return (int)eax;
}

#if defined(__aarch64__)

__attribute__((naked)) int AsmSchedule(int QueueID __attribute__((unused)))
{
    __asm__ volatile(
        "sub sp, sp, #256\n"
        "stp x0, x1, [sp, #0]\n"
        "stp x2, x3, [sp, #16]\n"
        "stp x4, x5, [sp, #32]\n"
        "stp x6, x7, [sp, #48]\n"
        "stp x8, x9, [sp, #64]\n"
        "stp x10, x11, [sp, #80]\n"
        "stp x12, x13, [sp, #96]\n"
        "stp x14, x15, [sp, #112]\n"
        "stp x16, x17, [sp, #128]\n"
        "stp x18, x19, [sp, #144]\n"
        "stp x20, x21, [sp, #160]\n"
        "stp x22, x23, [sp, #176]\n"
        "stp x24, x25, [sp, #192]\n"
        "stp x26, x27, [sp, #208]\n"
        "stp x28, x29, [sp, #224]\n"
        "str x30, [sp, #240]\n"
        "mov x1, x0\n"
        "mov x0, sp\n"
        "bl Schedule\n"
        "mov sp, x0\n"
        "ldp x0, x1, [sp, #0]\n"
        "ldp x2, x3, [sp, #16]\n"
        "ldp x4, x5, [sp, #32]\n"
        "ldp x6, x7, [sp, #48]\n"
        "ldp x8, x9, [sp, #64]\n"
        "ldp x10, x11, [sp, #80]\n"
        "ldp x12, x13, [sp, #96]\n"
        "ldp x14, x15, [sp, #112]\n"
        "ldp x16, x17, [sp, #128]\n"
        "ldp x18, x19, [sp, #144]\n"
        "ldp x20, x21, [sp, #160]\n"
        "ldp x22, x23, [sp, #176]\n"
        "ldp x24, x25, [sp, #192]\n"
        "ldp x26, x27, [sp, #208]\n"
        "ldp x28, x29, [sp, #224]\n"
        "ldr x30, [sp, #240]\n"
        "add sp, sp, #256\n"
        "ret\n");
}

#elif defined(__arm__)

__attribute__((naked)) int AsmSchedule(int QueueID __attribute__((unused)))
{
    __asm__ volatile(
        "push {r0-r12, lr}\n"
        "mov r1, r0\n"
        "mov r0, sp\n"
        "bl Schedule\n"
        "mov sp, r0\n"
        "pop {r0-r12, lr}\n"
        "bx lr\n");
}

#elif defined(__i386__)

__attribute__((naked)) int AsmSchedule(int QueueID __attribute__((unused)))
{
    __asm__ volatile(
        "pushl %eax\n"
        "pushl %ebx\n"
        "pushl %ecx\n"
        "pushl %edx\n"
        "pushl %esi\n"
        "pushl %edi\n"
        "pushl %ebp\n"
        "movl 0x20(%esp), %eax\n"
        "movl %esp, %ecx\n"
        "pushl %eax\n"
        "pushl %ecx\n"
        "call Schedule\n"
        "movl %eax, %esp\n"
        "popl %ebp\n"
        "popl %edi\n"
        "popl %esi\n"
        "popl %edx\n"
        "popl %ecx\n"
        "popl %ebx\n"
        "popl %eax\n"
        "ret\n");
}

#else

int AsmSchedule(int QueueID)
{
    uintptr_t sp;

#if defined(__x86_64__)
    __asm__ volatile("mov %%rsp, %0" : "=r"(sp));
#elif defined(__riscv)
    __asm__ volatile("mv %0, sp" : "=r"(sp));
#else
    sp = (uintptr_t)__builtin_frame_address(0);
#endif

    return (int)Schedule(sp, QueueID);
}

#endif

#endif /* defined(__i386__) && !defined(RDTSC_TEST_FORCE_C) */
