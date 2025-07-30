#ifndef MEM_ALLOC_INTERNALS_H
#define MEM_ALLOC_INTERNALS_H

#include <stddef.h>

// header metadata for simple memory block
typedef struct Block
{
    size_t size;
    int free;
    int is_mmap;
    struct Block *next, *prev;
} Block;

//memory alignment for allocated heap segment
#define ALIGNMENT 8
#define ALIGN(size) ((size + ALIGNMENT -1) & ~(ALIGNMENT -1))

//memory alignment with custom width
#define ALIGN_UP(size, pool) ((size + pool -1) & ~(pool -1))


//minimum block size i.e. for memory alignment in heap
#define MIN_BLOCK_SIZE 8

//==================== Block freelist implementation =============

// default page size for memory allocate using sbrk
#define PAGE_SIZE 4096

//function to find first-fit in the block freelist
Block* find_fit(size_t);

// allocate new block of given size
Block* allocate_block(size_t);

// split a larger block into two smaller ones in linked list
void split_block(Block*, size_t);

// coalesce adjecent free blocks
void coalesce(Block*);

// free a block in its metadata
void free_block(Block*);

//===================== MMAP allocation ============================

//mmap threshold limit
#define MMAP_THRESHOLD 128 * 1024

// memory allocation via mmap for larger chunks greater than MMAP_THRESHOLD
void* mmap_alloc(size_t);

// free mmap memory allocaton region
void mmap_free(Block*);

#endif