#ifndef THREADS_H
#define THREADS_H

#define __USE_GNU
#include <sched.h>
#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>

typedef struct {

  // defaults
  int id;
  pthread_barrier_t* barrier;
  
  // inputs
  int start_line_count; 
  int end_line_count;
  int avg_seq_len;
  int ht_size;
  int data_size;
  int k;
  char* path;

  // outputs
  uint64_t avg_cpo;
  uint64_t avg_tc;
  uint64_t avg_collisions;
  double   avg_fill;

} ThreadArgs;

typedef void (*ThreadHandler)(ThreadArgs*);

typedef struct {

  pthread_t* threads;
  pthread_attr_t* attrs;
  cpu_set_t cpuset;
  int num_threads;
  pthread_barrier_t barrier;

  ThreadArgs* args; // this a list: arguments for each threads. 
} Threads;

void initThreads(Threads* ts, int num)
{
  ts->num_threads = num;
  ts->threads = (pthread_t*) malloc(sizeof(pthread_t) * num);
  ts->attrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t) * num);
  ts->args = (ThreadArgs* )malloc(sizeof(ThreadArgs) * num);
  pthread_barrier_init(&ts->barrier, NULL, num); // Initialize the barrier
}

void destroyThreads(Threads* ts)
{
  free(ts->threads);
  free(ts->attrs);
  free(ts->args);
  pthread_barrier_destroy(&ts->barrier);
}

// Pass in a handler function and a list of ThreadArgs. Each handler will receives it own correspond args  
void runThreads(Threads* ts, ThreadHandler handler) 
{
    cpu_set_t* cpu = &ts->cpuset;
    for (int i = 0; i < ts->num_threads; ++i) {
        CPU_ZERO(cpu);
        CPU_SET(i, cpu);  // Set affinity to CPU core i
        pthread_attr_init(&(ts->attrs[i]));
        pthread_attr_setaffinity_np(&(ts->attrs[i]), sizeof(cpu_set_t), cpu);
        
        // create and run
        ts->args[i].id = i;  
        ts->args[i].barrier = &ts->barrier;
        pthread_create(&ts->threads[i], &ts->attrs[i], (void* (*) (void*)) handler, (void*) &ts->args[i]);
    }
}

void waitThreads(Threads* ts)
{
  for (int i = 0; i < ts->num_threads; ++i) {
    pthread_join(ts->threads[i], NULL);
  }    
}

#endif
