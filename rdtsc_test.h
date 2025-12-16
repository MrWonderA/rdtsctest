#ifndef RDTSC_TEST_H
#define RDTSC_TEST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef long long DLONG;

DLONG _RDTSC(void);
DLONG _LMULDWORD(DLONG value, int mul);
int _EXECMASM(void);
int AsmSchedule(int QueueID);

#ifdef __cplusplus
}
#endif

#endif /* RDTSC_TEST_H */
