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
  uint64_t size; 
} Data;

void initData(Data* data, uint64_t size)
{
  data->content = (Datatype*) aligned_alloc(4096, sizeof(Datatype) * size);
  data->size = size;
  data->len = 0;
} 

void insertData(Data* data, Datatype ele) 
{
  if(data->len < data->size)
  {
    data->content[data->len] = ele;
    data->len++;
  }
  else {
    printf("ERROR: data is full, insertion failed\n");
  }

}

void clearData(Data* data) 
{
  data->len = 0;
}

void destroyData(Data* data)
{
  free(data->content);
}


double zipf(int rank, double alpha) {
    return 1.0 / pow( (double) rank, alpha);
}

void generate_zipf_data(Data* data, double alpha) {
    double sum = 0.0;
    uint64_t i;
    uint64_t n = data->size;

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
                insertData(data, j);
                break;
            }
        }
    }
}



#endif
