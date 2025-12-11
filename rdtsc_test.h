#ifndef RDTSC_TEST_H
#define RDTSC_TEST_H

#include <stdint.h>

/*
 * rdtsc_test.h - CPU性能测试库头文件
 * 
 * 提供跨平台的CPU周期计数、乘法运算和性能校准接口
 * 支持x86和ARM架构
 */

/* CPU周期计数器读取 - 返回CPU执行的总周期数 */
uint64_t _RDTSC(void);

/* 32位 × 32位 → 64位乘法 */
uint64_t _LMULDWORD(uint32_t a, uint32_t b);

/* 确定性加减循环用于性能校准 */
void _EXECMASM(void);

/* 寄存器保存与函数调用包装器 */
uint64_t AsmSchedule(void (*func_ptr)(void));

#endif /* RDTSC_TEST_H */
