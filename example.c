#include <stdio.h>
#include <inttypes.h>
#include "rdtsc_test.h"

void Schedule(void *stack_ptr, void *param) {
    printf("Schedule: stack=%p, param=%p\n", stack_ptr, param);
}

void example_timing(void) {
    printf("=== Example 1: High-Precision Timing ===\n");
    
    uint64_t start = _RDTSC();
    
    volatile int sum = 0;
    for (int i = 0; i < 10000; i++) {
        sum += i;
    }
    
    uint64_t end = _RDTSC();
    uint64_t elapsed = end - start;
    
    printf("Loop executed in %" PRIu64 " cycles\n", elapsed);
    printf("Result: %d\n\n", sum);
}

void example_multiplication(void) {
    printf("=== Example 2: 64-bit Multiplication ===\n");
    
    uint32_t a = 1000000;
    uint32_t b = 2000000;
    uint64_t result = _LMULDWORD(a, b);
    
    printf("%u * %u = %" PRIu64 "\n", a, b, result);
    
    uint32_t max32 = 0xFFFFFFFF;
    uint64_t max_result = _LMULDWORD(max32, max32);
    printf("0x%X * 0x%X = 0x%" PRIX64 "\n\n", max32, max32, max_result);
}

void example_calibration(void) {
    printf("=== Example 3: Calibration Loop ===\n");
    
    uint64_t cycles[5];
    
    for (int i = 0; i < 5; i++) {
        uint64_t start = _RDTSC();
        _EXECMASM();
        uint64_t end = _RDTSC();
        cycles[i] = end - start;
    }
    
    printf("Calibration loop timings (5 runs):\n");
    for (int i = 0; i < 5; i++) {
        printf("  Run %d: %" PRIu64 " cycles\n", i + 1, cycles[i]);
    }
    printf("\n");
}

void example_scheduling(void) {
    printf("=== Example 4: Register-Preserving Call ===\n");
    
    int my_data = 42;
    printf("Calling AsmSchedule with param=%d\n", my_data);
    AsmSchedule(&my_data);
    printf("AsmSchedule returned successfully\n\n");
}

int main(void) {
    printf("rdtsctest Library Examples\n");
    printf("==========================\n\n");
    
    example_timing();
    example_multiplication();
    example_calibration();
    example_scheduling();
    
    printf("All examples completed!\n");
    return 0;
}
