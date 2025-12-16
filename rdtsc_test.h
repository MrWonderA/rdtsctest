#ifndef RDTSC_TEST_H
#define RDTSC_TEST_H

#include <stdint.h>

/*
 * rdtsc_test.h - CPU性能测试库头文件
 * 
 * 提供跨平台的CPU周期计数、乘法运算和性能校准接口
 * 支持x86和ARM架构
 * 
 * 函数声明与汇编版本兼容，返回类型使用long long(int64_t)
 */

/* CPU周期计数器读取 - 返回CPU执行的总周期数 */
long long _RDTSC(void);

/* 32位 × 32位 → 64位乘法 */
long long _LMULDWORD(long long a, int b);

/* 确定性加减循环用于性能校准 - 返回执行循环次数 */
int _EXECMASM(void);

/* 队列调度函数 - 根据QueueID执行调度操作 */
int AsmSchedule(int QueueID);

#endif /* RDTSC_TEST_H */
