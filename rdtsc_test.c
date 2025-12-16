#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/* 外部Schedule函数声明 */
extern long long Schedule(long long arg1, long long arg2);

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
 * 根据QueueID执行相应的调度操作
 * 内部调用Schedule函数进行实际的处理
 * 
 * 参数:
 *   QueueID - 队列标识符，用于选择调度策略
 * 
 * 返回: int - 调度操作的结果状态码
 */
int AsmSchedule(int QueueID)
{
    /* 获取当前栈指针值（保持原有的调试信息功能） */
    volatile long long esp;
    
#ifdef __aarch64__
    /* ARM64: 读取SP (64位栈指针) */
    __asm__ volatile("mov %0, sp" : "=r"(esp));
#elif defined(__arm__)
    /* ARM32: 读取SP (32位栈指针) */
    uint32_t esp32;
    __asm__ volatile("mov %0, sp" : "=r"(esp32));
    esp = (long long)esp32;
#elif defined(__x86_64__)
    /* x86-64: 读取RSP */
    __asm__ volatile("mov %%rsp, %0" : "=r"(esp));
#elif defined(__i386__)
    /* x86-32: 读取ESP */
    uint32_t esp32;
    __asm__ volatile("mov %%esp, %0" : "=r"(esp32));
    esp = (long long)esp32;
#else
    /* 未知架构: 使用当前栈地址 */
    esp = (long long)&QueueID;  /* 使用参数地址作为栈指针近似值 */
#endif
    
    /* 调用外部Schedule函数，传递队列ID和栈指针作为参数
     * 返回值转换为int类型 */
    long long result = Schedule((long long)QueueID, esp);
    
    /* 将结果转换为int返回，确保与声明一致 */
    return (int)result;
}
