#include <stdint.h>
#include <stddef.h>

/* 外部Schedule函数声明 */
extern uint64_t Schedule(uint64_t arg1, uint64_t arg2);

/*
 * _RDTSC - CPU周期计数器读取
 * 
 * ARM架构: 使用MRS指令读取PMCCNTR_EL0寄存器获取64位周期计数
 * x86架构: 使用RDTSC指令获取64位时间戳计数
 * 
 * 返回: uint64_t - CPU执行的总周期数
 */
uint64_t _RDTSC(void)
{
#ifdef __aarch64__
    /* ARM64 (ARMv8): 读取PMCCNTR_EL0性能计数寄存器 */
    uint64_t cycles;
    __asm__ volatile("mrs %0, pmccntr_el0" : "=r"(cycles));
    return cycles;
#elif defined(__arm__)
    /* ARM32 (ARMv7): 使用MRC指令读取PMCCNTR
     * MRC p15, 0, Rd, c9, c13, 0 - 读取周期计数寄存器 */
    uint32_t cycles;
    __asm__ volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(cycles));
    return (uint64_t)cycles;
#elif defined(__i386__) || defined(__x86_64__)
    /* x86/x64: RDTSC指令 */
    uint32_t low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
#else
    /* 未知架构: 返回0 */
    return 0;
#endif
}

/*
 * _LMULDWORD - 32位 × 32位 → 64位乘法
 * 
 * 纯C语言实现，使用uint64_t避免溢出
 * 
 * 参数:
 *   a - 第一个32位被乘数
 *   b - 第二个32位乘数
 * 
 * 返回: uint64_t - 64位乘积
 */
uint64_t _LMULDWORD(uint32_t a, uint32_t b)
{
    /* 将两个uint32_t转换为uint64_t后相乘
     * 这样可以避免溢出，直接得到64位结果 */
    return (uint64_t)a * (uint64_t)b;
}

/*
 * _EXECMASM - 确定性加减循环用于性能校准
 * 
 * 执行固定次数(10000)的add/sub操作对，用于CPU频率校准和基准测试
 * 每次迭代执行两条相反的指令，便于性能分析
 */
void _EXECMASM(void)
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
}

/*
 * AsmSchedule - 寄存器保存与函数调用包装器
 * 
 * 原x86实现功能:
 * 1. 保存所有通用寄存器(EAX,EBX,ECX,EDX,ESI,EDI,EBP)
 * 2. 从栈中获取函数指针参数
 * 3. 调用Schedule函数，传递函数指针和当前栈指针
 * 4. 恢复所有寄存器
 * 
 * C语言实现策略:
 * - 利用C的函数调用约定自动保存/恢复寄存器
 * - 使用volatile确保编译器不优化掉关键操作
 * - 调用Schedule，传递函数指针和栈指针作为参数
 * 
 * 参数:
 *   func_ptr - 指向调度函数的函数指针
 * 
 * 返回: uint64_t - Schedule函数的返回值
 */
uint64_t AsmSchedule(void (*func_ptr)(void))
{
    /* 获取当前栈指针值 */
    volatile uint64_t esp;
    
#ifdef __aarch64__
    /* ARM64: 读取RSP (64位栈指针) */
    __asm__ volatile("mov %0, sp" : "=r"(esp));
#elif defined(__arm__)
    /* ARM32: 读取SP (32位栈指针) */
    uint32_t esp32;
    __asm__ volatile("mov %0, sp" : "=r"(esp32));
    esp = (uint64_t)esp32;
#elif defined(__x86_64__)
    /* x86-64: 读取RSP */
    __asm__ volatile("mov %%rsp, %0" : "=r"(esp));
#elif defined(__i386__)
    /* x86-32: 读取ESP */
    uint32_t esp32;
    __asm__ volatile("mov %%esp, %0" : "=r"(esp32));
    esp = (uint64_t)esp32;
#endif
    
    /* 调用外部Schedule函数，传递函数指针和栈指针作为参数
     * C的函数调用约定会自动保存和恢复寄存器 */
    uint64_t result = Schedule((uint64_t)(uintptr_t)func_ptr, esp);
    
    return result;
}
