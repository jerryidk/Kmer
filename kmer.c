
#include <stdio.h>
#include <stdint.h>

#include "data.h"
#include "hashtable.h"
#include "partition.h"
#include "reader.h"
#include "threads.h"

void thread_worker(ThreadArgs* arg)
{
  printf("worker %d started! start: %d end: %d\n", arg->id, arg->start_line_count, arg->end_line_count);
  // Create data
  Data data; 
  initData(&data, 100); 

  // Create reader
  KmerReader reader;
  initReaderConcurrent(&reader, "./data/sample.fastq", 5, arg->start_line_count, arg->end_line_count);
  // Bring data from disk to data
  uint64_t num = read_data(&reader, &data);
  
  printf("number of data read in: %lu\n", num);
  destroyReader(&reader);
  destroyData(&data);
}

int main(int argc, char** argv) 
{
  Threads ts;
  int num_threads = 2;
  initThreads(&ts, num_threads);

  ts.args[0].start_line_count = 0;
  ts.args[0].end_line_count = 3;
  ts.args[1].start_line_count = 4;
  ts.args[1].end_line_count = 7;

  runThreads(&ts, thread_worker);
  waitThreads(&ts);
  destroyThreads(&ts);
}
