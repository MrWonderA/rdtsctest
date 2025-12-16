#include <stdio.h>
#include <stdint.h>

/* 外部Schedule函数声明 */
extern int Schedule(int esp, int QueueID);

/* 包含我们的优化后的函数 */
long long _RDTSC(void);
long long _LMULDWORD(long long a, int b);
int _EXECMASM(void);
int AsmSchedule(int QueueID);

/* 简单的Schedule函数实现用于测试 */
int Schedule(int esp, int QueueID) {
    printf("Schedule called with args: esp=%d, QueueID=%d\n", esp, QueueID);
    return esp + QueueID;
}

int main(void) {
    printf("=== rdtsc_test 优化版本测试 ===\n\n");
    
    /* 测试 _RDTSC */
    printf("1. 测试 _RDTSC (CPU周期计数):\n");
    long long cycles1 = _RDTSC();
    printf("   开始时间戳: %lld\n", cycles1);
    
    /* 测试 _EXECMASM */
    printf("\n2. 测试 _EXECMASM (性能校准循环):\n");
    int exec_result = _EXECMASM();
    printf("   执行结果: %d\n", exec_result);
    
    long long cycles2 = _RDTSC();
    printf("   结束时间戳: %lld\n", cycles2);
    printf("   消耗周期数: %lld\n", cycles2 - cycles1);
    
    /* 测试 _LMULDWORD */
    printf("\n3. 测试 _LMULDWORD (32×32→64位乘法):\n");
    long long a = 12345;
    int b = 6789;
    long long product = _LMULDWORD(a, b);
    printf("   %lld × %d = %lld\n", a, b, product);
    printf("   验证: %lld × %d = %lld\n", (int32_t)a, b, (int64_t)(int32_t)a * (int32_t)b);
    
    /* 测试 AsmSchedule */
    printf("\n4. 测试 AsmSchedule (队列调度):\n");
    int queue_id = 42;
    int schedule_result = AsmSchedule(queue_id);
    printf("   队列ID: %d\n", queue_id);
    printf("   调度结果: %d\n", schedule_result);
    
    printf("\n=== 所有测试完成 ===\n");
    return 0;
}