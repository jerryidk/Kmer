
#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>


uint64_t rdtsc() {
    uint32_t low, high;
    asm volatile ("mfence\n\trdtsc" : "=a" (low), "=d" (high));
    return ((uint64_t)high << 32) | low;
}



#endif
