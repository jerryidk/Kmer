#ifndef SIMPLE_HT
#define SIMPLE_HT

/**
 Hashtable with linear collision
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "data.h"
// Define a structure for a key-value pair
typedef struct KeyValuePair {
    Datatype key;
    uint64_t value;
    uint8_t  occupied;
} KeyValuePair;

// Define the HashTable structure
typedef struct {
    KeyValuePair* table;    
    uint64_t count; // number of elements
    uint64_t size;  // max number of elements
    uint64_t collision_count;
} HashTable;

void printHashTable(HashTable *ht) 
{
  
 KeyValuePair kv;
 for(uint64_t i=0; i<ht->size; i++){
   kv = ht->table[i];
   if(kv.occupied)
     printf("index: %lu key: %lu value: %lu \n", i, kv.key, kv.value);
 }
  printf("HT -- Count: %lu Collision: %lu Capacity: %lu\n", ht->count, ht->collision_count, ht->size);
}

// Hash function
uint64_t hash(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

// Initialize the hash table
void initHashTable(HashTable* ht, uint64_t size) {
    ht->table =  (KeyValuePair*) aligned_alloc(4096, size * sizeof(KeyValuePair));
    
    ht->size = size;
    ht->count = 0;
    ht->collision_count = 0;

    for(int i=0; i<ht->size; i++)
        ht->table[i].occupied = 0;
}

void clearHashTable(HashTable* ht)
{
    ht->count = 0;
    ht->collision_count = 0;

    for(int i=0; i<ht->size; i++)
        ht->table[i].occupied = 0;
   
}

void destroyHashTable(HashTable* ht)
{
    free(ht->table);
}

void saveHashTable(HashTable* ht, char* filename)
{
    FILE *file = fopen(filename, "wb");
    KeyValuePair* kv;
    for(int i=0; i<ht->size; i++)
    {
        kv = &ht->table[i];
        if(kv->occupied)
        {
            fwrite(&kv->key, sizeof(kv->key), 1, file); 
            fwrite(&kv->value, sizeof(kv->value), 1, file); 
        }
    }
    fclose(file);
}

// sum up all the value in the HT
uint64_t aggregate(HashTable* ht)
{
    uint64_t sum = 0;
    KeyValuePair* kv;
    for(int i=0; i<ht->size; i++)
    {
        kv = &ht->table[i];
        if(kv->occupied)
            sum += kv->value;
    }

    return sum;
}

// Upsert operation: insert or update key-value pair
int upsert(HashTable* ht, Datatype key, uint64_t value) {

    if(ht->count == ht->size)
        return -1;

    // collision. linear probing find next spot in the array.  
    uint64_t start_index = hash(key) % ht->size; 
    uint64_t index = start_index;
    KeyValuePair* current;  
    while(1)  
    {
        current = &ht->table[index];
        if (!current->occupied) 
        {
            current->key = key;
            current->value = value;
            current->occupied = 1;
            ht->count++; 
            return value;
        }

        if(current->key == key){
            current->value += value;
            return value;
        } 

        index = (index + 1) % ht->size;
        ht->collision_count++;
        if(index == start_index)
        {
            printf("upsert failed can't find a spot for key %lu\n", key);
            return -1; 
        }
    }
}

// Retrieve value associated with a key
int get(HashTable* ht, Datatype key, uint64_t* value) {
    uint64_t start_index = hash(key) % ht->size; 
    uint64_t index = start_index;
    KeyValuePair* current; 
    while(1)  
    {
        current = &ht->table[index];
    
        if(current->key == key){
            *value = current->value;
            return 1;
        } 

        index = (index + 1) % ht->size;
        if(index == start_index)
            return -1; 
    }
}


#endif
