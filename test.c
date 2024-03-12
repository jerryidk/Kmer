
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include "data.h"
#include "hashtable.h"
#include "partition.h"

char *Usage_str =
    "Usage: %s [-h] help               \n"
    "          [-t] <ht_size>          \n" 
    "          [-n] <data_size>        \n"
    "          [-a] <zipf_alpha>       \n"
    "          [-r] <experiment times> \n"
    "          [-c] CSV output enable  \n";

void run(uint64_t n, double alpha, uint64_t ht_size, bool csv);

int main(int argc, char **argv)
{
  // default values
  uint64_t n = 100;
  uint64_t ht_size = 100;
  double alpha = 0.5;
  bool csv = false;

  int e = 1;

  int opt;

  // Parse command-line options
  while ((opt = getopt(argc, argv, "ht:n:a:e:c")) != -1)
  {
    switch (opt)
    {
    case 'h':
      printf(Usage_str, argv[0]);
      exit(EXIT_SUCCESS);
    case 'e':
      e = atoi(optarg);
      break;
    case 'c':
      csv = true;
    case 't':
      ht_size = atoi(optarg);
      break;
    case 'n':
      n = atoi(optarg);
      break;
    case 'a':
      alpha = atof(optarg);
      break;

    default:
      printf(Usage_str, argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  // Process non-option arguments, if any
  for (int i = optind; i < argc; i++)
  {
    printf("Non-option argument: %s\n", argv[i]);
    exit(EXIT_FAILURE);
  }

  run(n, alpha, ht_size, csv);
}

void run(uint64_t n, double alpha, uint64_t ht_size, bool csv)
{
  Partition p;

  // memory allocation & zipf data generation
  createPartition(&p, ht_size, n, alpha);

  // workload
  partitionInsert(&p);

  // print out stats
  printPartitionStats(&p, csv);

  // Clean up
  destroyPartition(&p);
}
