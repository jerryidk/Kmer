
#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>


uint64_t rdtsc() {
    uint32_t low, high;
    asm volatile ("mfence\n\trdtsc" : "=a" (low), "=d" (high));
    return ((uint64_t)high << 32) | low;
}

int countLines(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return -1; // Return -1 to indicate an error
    }

    int lines = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            lines++;
        }
    }

    fclose(file);
    return lines;
}

#endif
