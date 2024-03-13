
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
    "          [-N] <avg_seq_len>      \n";

void thread_worker(ThreadArgs *arg);

int main(int argc, char **argv)
{
  char *path = "./data/sample.fastq";
  uint64_t n = 100;
  uint64_t ht_size = 100;
  int k = 5;
  int num_threads = 2;
  int avg_seq_len = 200;

  int opt;

  // Parse command-line options
  while ((opt = getopt(argc, argv, "hp:t:n:k:r:s:N:")) != -1)
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
    case 's':
      num_threads = atoi(optarg);
    case 'N':
      avg_seq_len = atoi(optarg);
    default:
      printf(Usage_str, argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  for (int i = optind; i < argc; i++)
  {
    printf("Non-option argument: %s\n", argv[i]);
    exit(EXIT_FAILURE);
  }

  Threads ts;
  initThreads(&ts, num_threads);

  int total = countLines(path);
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
  KmerReader reader;
  initReaderConcurrent(&reader, args->path, args->k, args->start_line_count, args->end_line_count);

  int num_parts = estimate_partition_num(
      args->end_line_count - args->start_line_count + 1,
      args->avg_seq_len,
      args->k,
      args->data_size);

  printf("worker %d started! num_partition created: %d sl: %d, el: %d\n",
         args->id,
         num_parts,
         args->start_line_count, args->end_line_count);

  Partition *ps = (Partition *)malloc(sizeof(Partition) * num_parts);
  for (int i = 0; i < num_parts; i++)
  {
    createParititonReaderThreaded(&ps[i], args->ht_size, args->data_size, &reader, args->id);
    partitionInsert(&ps[i]);
    printPartitionStats(&ps[i], false);
    destroyPartition(&ps[i]);
  }
}