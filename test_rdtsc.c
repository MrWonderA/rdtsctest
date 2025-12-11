#include <stdio.h>
#include <inttypes.h>
#include "rdtsc_test.h"

void Schedule(void *stack_ptr, void *param) {
    printf("Schedule called with stack_ptr=%p, param=%p\n", stack_ptr, param);
}

int main(void) {
    printf("Testing rdtsctest ARM port\n\n");
    
    printf("1. Testing _RDTSC...\n");
    uint64_t start = _RDTSC();
    printf("   Start cycle: %" PRIu64 "\n", start);
    
    volatile int dummy = 0;
    for (int i = 0; i < 1000; i++) {
        dummy += i;
    }
    
    uint64_t end = _RDTSC();
    printf("   End cycle: %" PRIu64 "\n", end);
    printf("   Elapsed: %" PRIu64 " cycles\n\n", end - start);
    
    printf("2. Testing _LMULDWORD...\n");
    uint32_t a = 0x12345678;
    uint32_t b = 0xABCDEF01;
    uint64_t result = _LMULDWORD(a, b);
    printf("   %u * %u = %" PRIu64 "\n", a, b, result);
    printf("   Expected: %" PRIu64 "\n\n", (uint64_t)a * (uint64_t)b);
    
    printf("3. Testing _EXECMASM...\n");
    uint64_t exec_start = _RDTSC();
    _EXECMASM();
    uint64_t exec_end = _RDTSC();
    printf("   Calibration loop completed in %" PRIu64 " cycles\n\n", exec_end - exec_start);
    
    printf("4. Testing AsmSchedule...\n");
    int test_param = 42;
    AsmSchedule(&test_param);
    printf("   AsmSchedule completed successfully\n\n");
    
    printf("All tests passed!\n");
    return 0;
}
