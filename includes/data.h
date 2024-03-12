#ifndef DATA_H
#define DATA_H

#include <stdint.h> 
#include <math.h>
#include <time.h>
#include <stdlib.h>

typedef uint64_t Datatype;

typedef struct {
  Datatype* content;
  uint64_t len;
} Data;

void initData(Data* data, uint64_t len)
{
  data->content = (Datatype*) aligned_alloc(4096, sizeof(Datatype) * len);
  data->len = len;
} 

void destroyData(Data* data)
{
  free(data->content);
}


double zipf(int rank, double alpha) {
    return 1.0 / pow( (double) rank, alpha);
}

void generate_zipf_data(uint64_t n, double alpha, Datatype* data) {
    double sum = 0.0;
    uint64_t i;

    // Calculate normalization constant
    for (i = 1; i <= n; i++) {
        sum += zipf(i, alpha);
    }

    // Generate data points
    srand(time(NULL)); // Seed the random number generator
    for (i = 0; i < n; i++) {
        double r = ((double)rand() / (RAND_MAX)) * sum;
        double partial_sum = 0.0;
        int j;
        for (j = 1; j <= n; j++) {
            partial_sum += zipf(j, alpha);
            if (partial_sum >= r) {
                data[i] = j;
                break;
            }
        }
    }
}



#endif
