#ifndef PARTITION_H
#define PARTITION_H

#include "hashtable.h"
#include "data.h"
#include "util.h"

typedef struct {
  uint64_t cpo;
  uint64_t total_cycles;
} Stats;

typedef struct {
    HashTable* ht;
    Data* data;
    Stats stat;
} Partition;

void initPartition(Partition* p, HashTable* ht, Data* data) 
{ 
  p->ht = ht;
  p->data = data; 
}

void partitionInsert(Partition* p)
{
  Data* data = p->data;
  HashTable* ht = p->ht;

  uint64_t s = rdtsc();

  for(uint64_t i = 0; i < data->len; i++)
  {
    if(upsert(ht, data->content[i], 1)<0){
      printf("Error, upsert failed! \n");
      return;
    } 
  } 

  p->stat.total_cycles  = rdtsc() - s;
  p->stat.cpo  = (uint64_t) (p->stat.total_cycles / data->len);
  
} 

#endif
