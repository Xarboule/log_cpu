#ifndef MSR_UTILS
#define MSR_UTILS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#define MAX_MSR_PATH    32

void write_msr(int core, long addr, uint64_t val);

uint64_t read_msr(int core, long addr);

static int msr_dev = 0;

#endif
