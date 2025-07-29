#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/mem_alloc.h"
#include "../include/mem_alloc_internals.h"


void* mem_alloc(size_t size)
{
    if (size <= 0)
        return NULL;
    if (size > MMAP_THRESHOLD)
    {
        return mmap_alloc(ALIGN(size));
    }
    Block* block = find_fit(size);
    if (!block)
    {
        block = allocate_block(ALIGN(size));
        if (!block)
            return NULL;
    }
    else
    {
        split_block(block, ALIGN(size));
        block->free = 0;
    }
    
    return (void*)(block + 1);
};

void* mem_calloc(int num, size_t size)
{
    if (size <= 0 || num <= 0)
        return NULL;
    size_t total_size = num * size;
    void* ptr = mem_alloc(total_size);
    if (!ptr)
        return NULL;
    memset(ptr, 0, total_size);
    return ptr;
};

void* mem_realloc(void* ptr, size_t size)
{
    if (!ptr)
        return mem_alloc(size);
    if (size <= 0)
    {
        free_alloc(ptr);
        return NULL;
    }
    Block* block = (Block*)ptr -1;
    if (block->size >= size)
        return ptr;
    if (block->next->free && (block->size + sizeof(Block) + block->next->size >= size))
    {
        block->size += sizeof(Block) + block->next->size;
        block->next = block->next->next;
        if (block->next)
            block->next->prev = block;
        split_block(block, size);
        return ptr;
    }
    void* new_ptr = mem_alloc(size);
    if (!new_ptr)
        return NULL;
    memcpy(new_ptr, ptr, block->size);
    free_alloc(ptr);
    return new_ptr;
};

void free_alloc(void *memptr)
{
    if (!memptr)
        return;
    Block* block = (Block*)memptr -1;
    if (block->free)
    {
        fprintf(stderr, "Double free detected: Can't perform this actionl!\n");
        exit(EXIT_FAILURE);
    }
    if (block->is_mmap)
    {
        mmap_free(block);
        return;
    }
    free_block(block);
};