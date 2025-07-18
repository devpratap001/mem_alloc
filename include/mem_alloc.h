#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

#include <stddef.h>

// header metadata for simple memory block
typedef struct Block
{
    size_t size;
    int free;
    struct Block *next, *prev;
} Block;

//minimum block size i.e. for memory alignment in heap
#define MIN_BLOCK_SIZE 8

//allocate dynamic memory on heap segment
void* mem_alloc(size_t);

//allocate and initialize dynamic memory to zero using calloc
void* mem_calloc(int, size_t);

//reallocate previously allocated memory
void* mem_realloc(void*, size_t);

//free dynamic memory allocated using mem_alloc
void free_alloc(void*);

#endif