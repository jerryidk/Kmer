
#include <stdio.h>
#include <stdint.h>

#include "data.h"
#include "hashtable.h"
#include "partition.h"

void run(uint64_t n, double alpha, uint64_t ht_size)
{
  // Generate some data
  Data data; 
  initData(&data, n); 
  generate_zipf_data(n, alpha, data.content);

  // Create hashtable
  HashTable ht;
  initHashTable(&ht, ht_size);

  Partition p;
  initPartition(&p, &ht, &data);
  
  partitionInsert(&p);

  printHashTable(&ht);
  printf("Workload --  total cycles: %lu cpo: %lu data size: %lu alpha: %f\n", 
    p.stat.total_cycles, 
    p.stat.cpo, 
    n, alpha);
  
  destroyData(&data);
  destroyHashTable(&ht);
}

int main() 
{
  uint64_t n = 100; 
  uint64_t ht_size = 100; 
  double alpha = 0.5;

  for(int i = 5; i < 15; i++)
  { 
    n = (1 << i);
    run(n, alpha, 2*n);
  }
}
