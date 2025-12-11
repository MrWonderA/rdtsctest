#ifndef RDTSC_TEST_H
#define RDTSC_TEST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t _RDTSC(void);

uint64_t _LMULDWORD(uint32_t low, uint32_t high);

void _EXECMASM(void);

void AsmSchedule(void *param);

extern void Schedule(void *stack_ptr, void *param);

#ifdef __cplusplus
}
#endif

#endif
