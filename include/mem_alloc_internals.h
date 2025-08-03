#ifndef MEM_ALLOC_INTERNALS_H
#define MEM_ALLOC_INTERNALS_H

#include <stddef.h>

// track heap start and heap end during each sbrk call
extern void* heap_start, *heap_end;

// header metadata for simple memory block
typedef struct Block
{
    size_t size;
    int free;                   //free | prev_free; lowest bit for prev_free
    int is_mmap;
    struct Block *next, *prev;
} Block;

//bits in free that represent curr_free and prev_free
#define CURR_FREE (1<<1)
#define PREV_FREE (1<<0)

// function to set prev free flag in Block->free
void set_prev_free(Block*);     // set prev_free flag to 1
void set_prev_full(Block*);     // set prev_free flag to 0

// footer metadata for every memory block
typedef struct Blockfooter
{
    size_t size;
}Blockfooter;

//memory alignment for allocated heap segment
#define ALIGNMENT 8
#define ALIGN(size) ((size + ALIGNMENT -1) & ~(ALIGNMENT -1))

//memory alignment with custom width
#define ALIGN_UP(size, pool) ((size + pool -1) & ~(pool -1))


//minimum block size i.e. for memory alignment in heap
#define MIN_BLOCK_SIZE sizeof(Block) + sizeof(Blockfooter) + ALIGNMENT

//==================== Block freelist implementation =============

// default page size for memory allocate using sbrk
#define PAGE_SIZE 4096

//function to find first-fit in the block freelist
Block* find_fit(size_t);

// allocate new block of given size
Block* allocate_block(size_t);

// remove a block from our general fallback freelist
void remove_from_freelist(Block*);

// split a larger block into two smaller ones in linked list
void split_block(Block*, size_t);

// coalesce adjecent free blocks
Block* coalesce(Block*);

// free a block in its metadata
void free_block(Block*);

//===================== MMAP allocation ============================

//mmap threshold limit
#define MMAP_THRESHOLD 128 * 1024

// memory allocation via mmap for larger chunks greater than MMAP_THRESHOLD
void* mmap_alloc(size_t);

// free mmap memory allocaton region
void mmap_free(Block*);

//===================== heap memory print for debugging ==============

void print_heap_status(void);

#endif