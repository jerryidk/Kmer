#ifndef PARTITION_H
#define PARTITION_H

#include "stdbool.h"
#include "hashtable.h"
#include "data.h"
#include "util.h"

typedef struct
{
  uint64_t cpo;
  uint64_t total_cycles;
  double alpha;
  double fill_factor;
} Stats;

typedef struct
{
  HashTable *ht;
  Data *data;
  Stats stat;
} Partition;

void initPartition(Partition *p, HashTable *ht, Data *data)
{
  p->ht = ht;
  p->data = data;
}

void createPartition(Partition *p, uint64_t ht_size, uint64_t data_size, double alpha)
{
  p->ht = (HashTable *)malloc(sizeof(HashTable));
  initHashTable(p->ht, ht_size);

  p->stat.alpha = alpha;
  p->data = (Data *)malloc(sizeof(Data));
  initData(p->data, data_size);

  generate_zipf_data(data_size, alpha, p->data->content);
}

void destroyPartition(Partition *p)
{
  destroyHashTable(p->ht);
  free(p->ht);
  destroyData(p->data);
  free(p->data);
}

void printPartitionStats(Partition *p, bool csv)
{

  if (csv)
  {
    printf("%lu, %lu, %f, %lu, %lu, %lu, %f\n",
           p->data->len,
           p->ht->size,
           p->stat.alpha,
           p->stat.total_cycles,
           p->stat.cpo,
           p->ht->collision_count,
           p->stat.fill_factor);
  }
  else
  {

    printf(
        "Partition info                 \n"
        "             -- data size: %lu \n"
        "             -- ht size  : %lu \n"
        "             -- alpha    : %f  \n"
        "result                         \n"
        "             -- total cycles     : %lu \n"
        "             -- cycle per op     : %lu \n"
        "             -- ht collisions    : %lu \n"
        "             -- ht count         : %lu \n"
        "             -- fill factor      : %f  \n",
        p->data->len,
        p->ht->size,
        p->stat.alpha,
        p->stat.total_cycles,
        p->stat.cpo,
        p->ht->collision_count,
        p->ht->count,
        p->stat.fill_factor);
  }
}

void partitionSelfTest(Partition *p)
{
  uint64_t l = aggregate(p->ht);
  uint64_t r = p->data->len;
  if (l != r)
  {
    printf("ERROR: Aggregation: %lu != Data len: %lu\n", l, r);
  }
}

void partitionInsert(Partition *p)
{
  Data *data = p->data;
  HashTable *ht = p->ht;

  uint64_t s = rdtsc();

  for (uint64_t i = 0; i < data->len; i++)
  {
    if (upsert(ht, data->content[i], 1) < 0)
    {
      printf("Error, upsert failed! \n");
      return;
    }
  }

  p->stat.total_cycles = rdtsc() - s;
  p->stat.cpo = (uint64_t)((double) p->stat.total_cycles / data->len);
  p->stat.fill_factor = (double)p->ht->count / p->ht->size;

  partitionSelfTest(p); 
}

void partitionQuery(Partition *p)
{
  Data *data = p->data;
  HashTable *ht = p->ht;

  uint64_t s = rdtsc();
  uint64_t v;
  for (uint64_t i = 0; i < data->len; i++)
  {
    if (get(ht, data->content[i], &v) < 0)
    {
      printf(" get: key %lu not found! \n", data->content[i]);
    }
  }

  p->stat.total_cycles = rdtsc() - s;
  p->stat.cpo = (uint64_t)(p->stat.total_cycles / data->len);
  p->stat.fill_factor = (double)p->ht->count / p->ht->size;

  partitionSelfTest(p); 
}

#endif
