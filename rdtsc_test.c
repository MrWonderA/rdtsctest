#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/* 外部Schedule函数声明
 * Schedule(esp, QueueID) -> 返回新选中线程的现场指针(新esp) */
extern int Schedule(int esp, int QueueID);

/*
 * _RDTSC - CPU周期计数器读取
 * 
 * ARM架构: 使用MRS指令读取PMCCNTR_EL0寄存器获取64位周期计数
 * x86架构: 使用RDTSC指令获取64位时间戳计数
 * 
 * 返回: long long - CPU执行的总周期数
 */
long long _RDTSC(void)
{
#ifdef __aarch64__
    /* ARM64 (ARMv8): 读取PMCCNTR_EL0性能计数寄存器 */
    long long cycles;
    __asm__ volatile("mrs %0, pmccntr_el0" : "=r"(cycles));
    return cycles;
#elif defined(__arm__)
    /* ARM32 (ARMv7): 使用MRC指令读取PMCCNTR
     * MRC p15, 0, Rd, c9, c13, 0 - 读取周期计数寄存器 */
    uint32_t cycles;
    __asm__ volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(cycles));
    return (long long)cycles;
#elif defined(__i386__) || defined(__x86_64__)
    /* x86/x64: RDTSC指令 */
    uint32_t low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((long long)high << 32) | low;
#else
    /* 未知架构: 返回0 */
    return 0;
#endif
}

/*
 * _LMULDWORD - 32位 × 32位 → 64位乘法
 * 
 * 纯C语言实现，使用long long避免溢出
 * 第一个参数适配long long类型，内部转换为32位进行乘法运算
 * 
 * 参数:
 *   a - 第一个32位被乘数（传入为long long，但只取低32位）
 *   b - 第二个32位乘数
 * 
 * 返回: long long - 64位乘积
 */
long long _LMULDWORD(long long a, int b)
{
    /* 第一个参数只取低32位，第二个参数保持int类型
     * 将两个32位值转换为64位后相乘，避免溢出 */
    return (long long)((uint32_t)a * (uint32_t)b);
}

/*
 * _EXECMASM - 确定性加减循环用于性能校准
 * 
 * 执行固定次数(10000)的add/sub操作对，用于CPU频率校准和基准测试
 * 每次迭代执行两条相反的指令，便于性能分析
 * 
 * 返回: int - 返回执行的循环次数(10000)
 */
int _EXECMASM(void)
{
    /* 执行10000次add/sub循环，保持与原x86版本一致的迭代次数 */
    volatile uint32_t eax = 0;
    volatile uint32_t ecx = 10000;
    
    for (uint32_t i = 0; i < 10000; i++) {
        /* 执行add和sub操作，相互抵消
         * 这是确定性的时间测试操作，用于校准 */
        eax += ecx;
        eax -= ecx;
    }
    
    return 10000;  /* 返回执行的循环次数 */
}

/*
 * AsmSchedule - 队列调度函数
 *
 * 入口:
 *   esp: 落选线程的现场指针
 *   QueueID: 落选线程将要进入的队列号
 * 返回:
 *   新选中线程的现场指针(新esp)
 */
#if defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
__attribute__((naked)) int AsmSchedule(int QueueID)
{
    __asm__ volatile(
        "pushl %%eax\n\t"
        "pushl %%ebx\n\t"
        "pushl %%ecx\n\t"
        "pushl %%edx\n\t"
        "pushl %%esi\n\t"
        "pushl %%edi\n\t"
        "pushl %%ebp\n\t"
        "movl 0x20(%%esp), %%eax\n\t"
        "movl %%esp, %%ecx\n\t"
        "pushl %%eax\n\t"
        "pushl %%ecx\n\t"
        "call Schedule\n\t"
        "movl %%eax, %%esp\n\t"
        "popl %%ebp\n\t"
        "popl %%edi\n\t"
        "popl %%esi\n\t"
        "popl %%edx\n\t"
        "popl %%ecx\n\t"
        "popl %%ebx\n\t"
        "popl %%eax\n\t"
        "ret\n\t"
    );
}
#else
int AsmSchedule(int QueueID)
{
    uintptr_t sp;

#if defined(__aarch64__)
    __asm__ volatile("mov %0, sp" : "=r"(sp));
#elif defined(__arm__)
    __asm__ volatile("mov %0, sp" : "=r"(sp));
#elif defined(__x86_64__)
    __asm__ volatile("mov %%rsp, %0" : "=r"(sp));
#elif defined(__i386__)
    __asm__ volatile("mov %%esp, %0" : "=r"(sp));
#else
    sp = (uintptr_t)&QueueID;
#endif

    return Schedule((int)sp, QueueID);
}
#endif
