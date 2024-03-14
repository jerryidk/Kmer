
#include <stdio.h>
#include <stdint.h>

#include "data.h"
#include "hashtable.h"
#include "partition.h"
#include "kmer_reader.h"
#include "threads.h"
#include "util.h"

char *Usage_str =
    "Usage: %s\n"
    "          [-h] help               \n"
    "          [-t] <ht_size>          \n"
    "          [-n] <data_size>        \n"
    "          [-p] <path>             \n"
    "          [-k] <kmer>             \n"
    "          [-s] <num_threads>      \n"
    "          [-N] <avg_seq_len>      \n"
    "          [-c] <line_count>       \n";

void thread_worker(ThreadArgs *arg);

int main(int argc, char **argv)
{
  char *path = "./data/sample.fastq";
  uint64_t n = 100;
  uint64_t ht_size = 100;
  int k = 5;
  int num_threads = 2;
  int avg_seq_len = 200;
  int lc = 100;
  int opt;

  // Parse command-line options
  while ((opt = getopt(argc, argv, "hp:t:n:k:r:s:c:N:")) != -1)
  {
    switch (opt)
    {
    case 'h':
      printf(Usage_str, argv[0]);
      exit(EXIT_SUCCESS);
    case 'p':
      path = optarg;
      break;
    case 't':
      ht_size = atoi(optarg);
      break;
    case 'n':
      n = atoi(optarg);
      break;
    case 'k':
      k = atoi(optarg);
      break;
    case 's':
      num_threads = atoi(optarg);
      break;
    case 'N':
      avg_seq_len = atoi(optarg);
      break;
    case 'c':
      lc = atoi(optarg);
      break;
    default:
      printf("unknown option: %s \n", optarg);
      exit(EXIT_FAILURE);
    }
  }

  for (int i = optind; i < argc; i++)
  {
    printf("Non-option argument: %s\n", argv[i]);
    exit(EXIT_FAILURE);
  }

#ifdef DEBUG
  printf(
      "Program Configuration: \n"
      "       path: %s \n"
      "       ht-size %lu \n"
      "       data-size %lu \n"
      "       num_threads %d \n"
      "       k %d \n"
      "       lc %d \n"
      "       seq_length %d \n",
      path, ht_size, n, num_threads, k, lc, avg_seq_len);
#endif

  Threads ts;
  initThreads(&ts, num_threads);

  int total = lc; // countLines(path);
  int pl = (total + num_threads - 1) / num_threads;
  int sl = 0;

  for (int i = 0; i < num_threads; i++)
  {
    ThreadArgs *args = &ts.args[i];
    args->data_size = n;
    args->ht_size = ht_size;
    args->k = k;
    args->path = path;
    args->start_line_count = sl; // calculate lines
    args->end_line_count = sl + pl;
    args->avg_seq_len = avg_seq_len;
    sl += pl;
  }

  runThreads(&ts, thread_worker);
  waitThreads(&ts);

  // results
  printf("Result ---------------- \n");
  double total_perf_score_in_mop = 0.0;
  double mop = 0.0;
  for (int i = 0; i < num_threads; i++)
  {
    ThreadArgs *args = &ts.args[i];
    mop = (2.6 * 1000.0) / args->avg_cpo;
    total_perf_score_in_mop += mop;
    printf("tid: %d, mop: %f, avg cpo: %lu, avg tc: %lu, avg collisions: %lu, avg fill: %f\n",
           args->id, mop, args->avg_cpo, args->avg_tc, args->avg_collisions, args->avg_fill);
  }

  printf(">>> Score: %f mop/s\n", total_perf_score_in_mop);

  destroyThreads(&ts);
}

int estimate_partition_num(int lc, int n, int k, int d)
{
  int alc = (lc + 4 - 1) / 4;     // since every 4 lines contains 1 sequence. we only get process 1/4 of the file.
  int nkpl = n - k + 1;           // estimate number of kmer per line set N to some reasonable number for each line.
  int num_parts = alc * nkpl / d; // estimated number of partitions needed for a given data size.

  return num_parts;
}

void thread_worker(ThreadArgs *args)
{

  // Calculate number of of partitions needed base on and

  int num_parts = estimate_partition_num(
      args->end_line_count - args->start_line_count + 1,
      args->avg_seq_len,
      args->k,
      args->data_size);

#ifdef DEBUG
  printf("worker %d started! num_partition created: %d sl: %d, el: %d\n",
         args->id,
         num_parts,
         args->start_line_count, args->end_line_count);
#endif

  KmerReader reader;
  initReaderConcurrent(&reader, args->path, args->k, args->start_line_count, args->end_line_count);

  Partition *ps = (Partition *)malloc(sizeof(Partition) * num_parts);
  Partition *p;

  int i;
  for (i = 0; i < num_parts; i++)
  {
    p = &ps[i];
    if (createParititonReaderThreaded(p, args->ht_size, args->data_size, &reader, args->id) < 1)
    {
#ifdef DEBUG
      printf("Partition %d not neccessary. \n", i);
#endif
      destroyPartition(p);
      break;
    }

#ifdef DEBUG
    printf("partition %d insertion started\n", i);
#endif

    partitionInsert(p);
    args->avg_cpo += p->stat.cpo;
    args->avg_tc += p->stat.total_cycles;
    args->avg_fill += p->stat.fill_factor;
    args->avg_collisions += p->ht->collision_count;

#ifdef DEBUG
    printPartitionStats(p, false);
#endif
    destroyPartition(p);
  }

  if (i > 0)
  {
    args->avg_cpo = args->avg_cpo / i;
    args->avg_tc = args->avg_tc / i;
    args->avg_collisions = args->avg_collisions / i;
    args->avg_fill = args->avg_fill / i;
  }

  free(ps);
  destroyReader(p->reader);
}