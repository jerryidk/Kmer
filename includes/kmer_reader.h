
#ifndef READER_H
#define READER_H

#include "data.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h> 

#define BUFFER_LEN 1024

typedef struct {

  FILE *file;
  char buffer[BUFFER_LEN];
  uint64_t buffer_pos; // buffer position in byte.
  uint64_t line_count; 
  uint8_t k;
  bool done; // done

  uint64_t start_line_count; 
  uint64_t end_line_count;

} KmerReader;

void initReader(KmerReader* reader, char* path, uint8_t k) 
{
  
  reader->start_line_count = 0;
  reader->end_line_count = UINT64_MAX;
  reader->k = k;
  reader->done = false;
  reader->line_count = 0;
  reader->buffer_pos = BUFFER_LEN - 1; // set to the end 
  reader->file = fopen(path, "r");
  
  if(reader->file == NULL)
  {
    printf("ERROR: file %s can't be open\n", path);
    return;
  }
}

// Used for Concurrency
void initReaderConcurrent(KmerReader* reader, char* path, uint8_t k, uint64_t start_line_count, uint64_t end_line_count) 
{

  if(start_line_count > end_line_count) 
  {
      printf("ERROR: start line >= end line \n");
      return;
  }
  initReader(reader, path, k); 
  reader->start_line_count = start_line_count;
  reader->end_line_count = end_line_count;

  int start_line = 0;
  while (start_line < start_line_count && fgets(reader->buffer, BUFFER_LEN, reader->file))
  {
      start_line++;
  }
}

void destroyReader(KmerReader* reader)
{
  fclose(reader->file);
}

// get a single kmer from reader 
Datatype read_one(KmerReader* reader) 
{
  uint64_t buffer_len = strlen(reader->buffer);
  if(reader->buffer_pos + reader->k >= buffer_len){

    // find new line
    while (fgets(reader->buffer, BUFFER_LEN, reader->file))
    {
      reader->line_count++;
      if(reader->line_count + reader->start_line_count >= reader->end_line_count)
        return reader->done = true;

      if(reader->line_count % 4 == 2)
      {
        reader->buffer_pos = 0;
        break;
      }
    }

    // loop was never entered.
    if(reader->buffer_pos != 0)
      return reader->done = true; 
  } 
  
  int start = reader->buffer_pos;
  uint8_t nt;
  uint64_t kmer = 0;
  char c;
  for(int i = 0; i < reader->k; i++) {
    c = reader->buffer[i + start];  

    switch (c) {
      case 'a':
      case 'A':
          nt = 0;
          break;
      case 'c':
      case 'C':
          nt = 1;
          break;
      case 'g':
      case 'G':
          nt = 2;
          break;
      case 't':
      case 'T':
          nt = 3;
          break;
      default:
          nt = 0;
          break;
    }
    kmer = (kmer | nt) << 2;
  }

  reader->buffer_pos++;
  return kmer;
}

// Fill dst through reader, return number of elements being read into dst.
uint64_t read_data(KmerReader* reader, Data* dst) 
{
  uint64_t i = 0;
  while(!reader->done && i < dst->size) {
    insertData(dst, read_one(reader));
    i++;
  }
}
#endif


