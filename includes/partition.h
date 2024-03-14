#ifndef PARTITION_H
#define PARTITION_H

#include "stdbool.h"
#include "hashtable.h"
#include "data.h"
#include "util.h"
#include "kmer_reader.h"
typedef struct
{
  uint64_t cpo;
  uint64_t total_cycles;
  uint64_t num_drop; // number of kv ht dropped. in case ht size is smaller than data size
  double alpha;
  double fill_factor;
} Stats;

typedef struct
{
  HashTable *ht;
  Data *data;
  Stats stat;
  KmerReader* reader;
  int tid;
} Partition;

void initPartition(Partition *p, uint64_t ht_size, uint64_t data_size)
{
  p->ht = (HashTable *)malloc(sizeof(HashTable));
  initHashTable(p->ht, ht_size);
  p->data = (Data *)malloc(sizeof(Data));
  initData(p->data, data_size);
}

void createPartition(Partition *p, uint64_t ht_size, uint64_t data_size, double alpha)
{
  initPartition(p, ht_size, data_size);
  generate_zipf_data(p->data, alpha);
  p->reader = NULL;
  p->stat.alpha = alpha;
  p->tid = 0;
}

int createParititonReaderThreaded(Partition *p, uint64_t ht_size, uint64_t data_size, KmerReader* reader, int tid)
{
  initPartition(p, ht_size, data_size);
  p->stat.alpha = -1.0; // absence
  p->reader = reader;
  p->tid = tid;
  return read_data(reader, p->data);  // fill data as much as 
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
    printf("%d, %lu, %lu, %f, %lu, %lu, %lu, %f\n",
           p->tid,
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
        "tid:%d,"
        "data_size:%lu,"
        "ht_size: %lu,"
        "alpha: %f,"
        "total_cycles: %lu,"
        "cycle_per_op: %lu,"
        "ht collisions: %lu,"
        "ht count: %lu,"
        "fill factor: %f,"
        "num drop: %lu\n",
        p->tid,
        p->data->len,
        p->ht->size,
        p->stat.alpha,
        p->stat.total_cycles,
        p->stat.cpo,
        p->ht->collision_count,
        p->ht->count,
        p->stat.fill_factor,
        p->stat.num_drop);
  }
}

void partitionSelfTest(Partition *p, uint64_t drop)
{
  uint64_t l = aggregate(p->ht);
  uint64_t r = p->data->len;
  if (l != (r - drop))
  {
    printf("ERROR: Aggregation: %lu != Data len: %lu Drop: %lu \n", l, r, drop);
    exit(EXIT_FAILURE);
  }
}

void partitionInsert(Partition *p)
{
  Data *data = p->data;
  HashTable *ht = p->ht;

  if(data->len == 0) 
  {
    return;
  }

  uint64_t num_drop = 0; 
  uint64_t s = rdtsc();

  for (uint64_t i = 0; i < data->len; i++)
  {
    if (upsert(ht, data->content[i], 1) < 0)
    {
      num_drop++; // insertion failed, means it is full.
    }
  }

  p->stat.total_cycles = rdtsc() - s;
  p->stat.cpo = (uint64_t)((double) p->stat.total_cycles / data->len);
  p->stat.fill_factor = (double)p->ht->count / p->ht->size;
  p->stat.num_drop = num_drop;

  partitionSelfTest(p, num_drop); 
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
}

#endif
