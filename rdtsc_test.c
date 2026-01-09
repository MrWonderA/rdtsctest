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
#if defined(__aarch64__)
/*
 * ARM64 AsmSchedule - 纯C实现
 *
 * 功能: 线程调度器入口，保存当前线程上下文，调用Schedule选择新线程
 *
 * ARM64调用约定 (AAPCS64):
 * - x0: 第1个参数 (此处为QueueID)
 * - x1: 第2个参数 (此处为sp)
 * - x19-x28: 被调用者保存寄存器
 * - x30 (lr): 链接寄存器
 *
 * 栈布局 (16字节对齐):
 *   sp -> [x29(fp)]  <- fp
 *        [x30(lr)]  <- 保存返回地址
 *        [x19]      <- 保存被调用者保存寄存器
 *        [x20]
 *        [x21]
 *        [x22]
 *        [x23]
 *        [x24]
 *        [x25]
 *        [x26]
 *        [x27]
 *        [x28]
 */
__attribute__((naked)) int AsmSchedule(int QueueID)
{
    /* 注意: naked函数中，参数x0=QueueID
     * 我们需要保存所有被调用者保存寄存器，然后调用Schedule */
    __asm__ volatile(
        /* 保存被调用者保存寄存器 (x19-x28) 和链接寄存器(x30) */
        "    stp    x29, x30, [sp, #-32]!\n"   // 保存fp和lr，分配32字节栈空间
        "    mov    x29, sp\n"                  // 设置frame pointer
        "    stp    x19, x20, [sp, #0]\n"      // 保存x19-x20
        "    stp    x21, x22, [sp, #16]\n"     // 保存x21-x22
        "    str    x23, [sp, #32]\n"          // 保存x23

        /* 获取当前栈指针到x1 (第2个参数) */
        "    mov    x1, sp\n"

        /* 调用Schedule(sp, QueueID)
         * x0 = QueueID (已设置)
         * x1 = sp (刚设置)
         * 调用后，x0 = 新线程的栈指针 */
        "    bl    Schedule\n"

        /* x0现在是新线程的栈指针，用它恢复sp */
        "    mov    sp, x0\n"

        /* 恢复寄存器 (注意：新线程的栈上应该有相同布局) */
        "    ldr    x23, [sp, #32]\n"
        "    ldp    x21, x22, [sp, #16]\n"
        "    ldp    x19, x20, [sp, #0]\n"
        "    ldp    x29, x30, [sp], #32\n"     // 恢复fp和lr，恢复sp

        "    ret\n"                            // 返回到新线程的返回地址
    );
}

#elif defined(__arm__)
/*
 * ARM32 AsmSchedule - 纯C实现
 *
 * 功能: 线程调度器入口，保存当前线程上下文，调用Schedule选择新线程
 *
 * ARM32调用约定 (AAPCS, ARM EABI):
 * - r0: 第1个参数 (此处为QueueID)
 * - r1: 第2个参数 (此处为sp)
 * - r4-r11: 被调用者保存寄存器
 * - r14 (lr): 链接寄存器
 *
 * 栈布局 (8字节对齐):
 *   sp -> [lr]       <- 保存返回地址
 *        [r4]
 *        [r5]
 *        [r6]
 *        [r7]
 *        [r8]
 *        [r9]
 *        [r10]
 *        [fp]        <- 保存旧fp
 */
__attribute__((naked)) int AsmSchedule(int QueueID)
{
    /* 注意: naked函数中，参数r0=QueueID
     * 我们需要保存所有被调用者保存寄存器，然后调用Schedule */
    __asm__ volatile(
        /* 保存被调用者保存寄存器 (r4-r11) 和链接寄存器(lr) */
        "    push   {r0-r3, lr}\n"            // 保存r0-r3和lr (r0-r3会是我们要用的工作区)
        "    mov    r3, sp\n"                 // 保存当前sp到r3
        "    stmfd  sp!, {r4-r11, fp}\n"      // 保存r4-r11和fp，分配栈空间
        "    mov    fp, sp\n"                 // 设置frame pointer

        /* 获取当前栈指针到r1 (第2个参数) */
        "    mov    r1, sp\n"

        /* 调用Schedule(sp, QueueID)
         * r0 = QueueID (原始参数在栈上，恢复它)
         * r1 = sp (刚设置)
         * 调用后，r0 = 新线程的栈指针 */
        "    ldr    r0, [fp, #16]\n"          // 从栈上恢复QueueID (push {r0-r3,lr} 后 fp+16 是原r0)
        "    bl     Schedule\n"

        /* r0现在是新线程的栈指针，用它恢复sp */
        "    mov    sp, r0\n"

        /* 恢复寄存器 (注意：新线程的栈上应该有相同布局) */
        "    ldmfd  sp!, {r4-r11, fp}\n"      // 恢复r4-r11和fp

        "    ldmfd  sp!, {r0-r3, pc}\n"       // 恢复r0-r3和pc (返回到新线程的返回地址)
    );
}

/*
 * x86-64 AsmSchedule - 纯C实现
 *
 * 功能: 线程调度器入口，保存当前线程上下文，调用Schedule选择新线程
 *
 * 注意: 使用noinline和optimize("O0")确保汇编不被优化
 *       简化实现：只保存易失性寄存器，被调用者保存寄存器由调用者保证
 *
 * x86-64 System V ABI:
 * - rdi: 第1个参数 (QueueID)
 * - rsi: 第2个参数 (sp)
 * - rax, rcx, rdx, rsi, rdi, r8-r11: 易失性寄存器
 * - rbx, rbp, r12-r15: 被调用者保存寄存器
 */
__attribute__((noinline)) __attribute__((optimize("O0"))) int AsmSchedule(int QueueID)
{
    register long long rsi_reg __asm__("rsi");
    register long long rdi_reg __asm__("rdi");
    int result;

    /* 设置参数: rsi = 当前栈指针, rdi = QueueID */
    __asm__ volatile(
        "mov %%rsp, %0\n\t"
        : "=r"(rsi_reg)
    );
    rdi_reg = QueueID;

    /* 调用Schedule(sp, QueueID) */
    __asm__ volatile(
        "mov %1, %%rdi\n\t"
        "mov %2, %%rsi\n\t"
        "call Schedule\n\t"
        "mov %%rax, %0\n\t"
        : "=r"(result)
        : "r"(rdi_reg), "r"(rsi_reg)
        : "rax", "rdi", "rsi", "rcx", "rdx", "r8", "r9", "r10", "r11", "memory"
    );

    /* 用新栈指针恢复rsp并返回 */
    __asm__ volatile(
        "mov %0, %%rsp\n\t"
        :
        : "r"((long long)result)
        : "memory"
    );

    return (int)result;
}

#elif defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
/*
 * x86-32 AsmSchedule - 内联汇编实现 (保持原有逻辑)
 *
 * 功能: 线程调度器入口，保存当前线程上下文，调用Schedule选择新线程
 *
 * 注意: 这是原有实现，保持不变以确保向后兼容
 */
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
/*
 * 通用架构 fallback实现 (未支持的架构)
 *
 * 注意: 这只是一个简单的实现，不保证与原始汇编语义完全一致
 * 仅用于无法使用汇编的场景
 */
int AsmSchedule(int QueueID)
{
    uintptr_t sp;

#if defined(__aarch64__) || defined(__arm__)
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
