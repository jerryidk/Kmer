
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "hashtable.h"
#include "kmer_reader.h"
#include "partition.h"
#include "threads.h"
#include "util.h"

char *Usage_str = "Usage: %s\n"
                  "          [-h] help               \n"
                  "          [-t] <ht_size>          \n"
                  "          [-n] <data_size>        \n"
                  "          [-p] <path>             \n"
                  "          [-k] <kmer>             \n"
                  "          [-s] <num_threads>      \n"
                  "          [-N] <avg_seq_len>      \n"
                  "          [-c] <line_count>       \n";

void thread_worker(ThreadArgs *arg);

int main(int argc, char **argv) {
  char *path = "./data/sample.fastq";
  uint64_t n = 100;
  uint64_t ht_size = 100;
  int k = 5;
  int num_threads = 2;
  int avg_seq_len = 200;
  int lc = 100;
  int opt;

  // Parse command-line options
  while ((opt = getopt(argc, argv, "hp:t:n:k:r:s:c:N:")) != -1) {
    switch (opt) {
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

  for (int i = optind; i < argc; i++) {
    printf("Non-option argument: %s\n", argv[i]);
    exit(EXIT_FAILURE);
  }

#ifdef DEBUG
  printf("Program Configuration: \n"
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

  for (int i = 0; i < num_threads; i++) {
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
  for (int i = 0; i < num_threads; i++) {
    ThreadArgs *args = &ts.args[i];
    mop = (2.6 * 1000.0) / args->avg_cpo;
    total_perf_score_in_mop += mop;
    printf("tid: %d, mop: %f, avg cpo: %lu, avg tc: %lu, avg collisions: %lu, "
           "avg fill: %f\n",
           args->id, mop, args->avg_cpo, args->avg_tc, args->avg_collisions,
           args->avg_fill);
  }

  printf(">>> Score: %f mop/s\n", total_perf_score_in_mop);

  destroyThreads(&ts);
}

int estimate_partition_num(int lc, int n, int k, int d) {
  int alc = (lc + 4 - 1) / 4; // since every 4 lines contains 1 sequence. we
                              // only get process 1/4 of the file.
  int nkpl = n - k + 1;       // estimate number of kmer per line set N to some
                              // reasonable number for each line.
  int num_parts =
      alc * nkpl /
      d; // estimated number of partitions needed for a given data size.

  return num_parts;
}

/*
*
* Disk -> Memory -> HT -> Disk
*
*/
void thread_worker(ThreadArgs *args) {

  // Calculate number of of partitions needed base on and

  int num_parts =
      estimate_partition_num(args->end_line_count - args->start_line_count + 1,
                             args->avg_seq_len, args->k, args->data_size);
  printf("number of paritions! %d \n", num_parts);
#ifdef DEBUG
  printf("worker %d started! num_partition created: %d sl: %d, el: %d\n",
         args->id, num_parts, args->start_line_count, args->end_line_count);
#endif

  KmerReader reader;
  initReaderConcurrent(&reader, args->path, args->k, args->start_line_count,
                       args->end_line_count);
  HashTable ht; 
  initHashTable(&ht, args->ht_size);
  Data data;
  initData(&data, args->data_size);
  char* filename = (char*) malloc(sizeof(char) * 15);

  uint64_t num_inserted = 0;

  uint64_t start = rdtsc();
  
  for (int i = 0; i < num_parts; i++) {

    clearData(&data);
    clearHashTable(&ht);
    if (read_data(&reader, &data) < 1) {
      printf("Reader exhausted! \n");
      break;
    }
    
    for (uint64_t j = 0; j < data.len; j++)
    {
      if (upsert(&ht, data.content[j], 1) < 0)
      {
        printf("HT full\n");
      }
      else
        num_inserted++;
    }
    sprintf(filename,"output-%d-%d.bin", i, args->id);
    saveHashTable(&ht,filename);
  }

  uint64_t total = rdtsc() - start;
  uint64_t cpo = total/num_inserted; 
  printf("Tid %d, num_inserted: %lu, total: %lu, cpo: %lu\n",args->id, num_inserted, total, cpo);

  args->avg_cpo = cpo;

  destroyHashTable(&ht);
  destroyData(&data);
  destroyReader(&reader);
  free(filename);
}
