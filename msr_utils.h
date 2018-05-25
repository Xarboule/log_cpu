#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#define MAX_MSR_PATH    32

void write_msr(int core, long addr, u_int64_t val);

u_int64_t read_msr(int core, long addr);

