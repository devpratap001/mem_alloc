#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../include/mem_alloc.h"

static Block *head = NULL, *tail = NULL;

Block* find_fit(size_t size);
Block* allocate_block(size_t size);
void split_block(Block*, size_t);
void coalesce_block(Block*);
void* mmap_alloc(size_t size);
void free_block(Block*);

void* mem_alloc(size_t size)
{
    if (size <= 0)
        return NULL;
    if (size > MMAP_THRESHOLD)
    {
        return mmap_alloc(size);
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
        split_block(block, size);
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

void* mmap_alloc(size_t size)
{
    size_t tot_size = sizeof(Block) + size;
    Block* ptr = mmap(NULL, tot_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED)
        {
            return NULL;
        }
        ptr->free = 0;
        ptr->next = NULL;
        ptr->prev = NULL;
        ptr->is_mmap = 1;
        ptr->size = size;
        return (void*)(ptr + 1);
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
        if (munmap((Block*)block, block->size + sizeof(Block)))
        {
            perror("Can't free memory");
            exit(EXIT_FAILURE);
        }
        return;
    }
    free_block(block);
};

Block* find_fit(size_t size)
{
    Block* curr = head;
    while(curr)
    {
        if (curr->free && curr->size >= size)
        {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
};

Block* allocate_block(size_t size)
{
    size_t total_size = sizeof(Block) + size;
    Block* block = sbrk(0);
    if (sbrk(ALIGN(total_size)) == (void*)-1)
        return NULL;
    block->size = size;
    block->free = 0;
    block->next = NULL;
    block->prev = NULL;
    block->is_mmap = 0;

    if (!head)
        head = block;
    if (tail)
    {
        tail->next = block;
        block->prev = tail;
    }
    
    tail = block;

    return block;
};

void split_block(Block* block, size_t size)
{
    if (block->size >= ALIGN(size) + sizeof(Block) + MIN_BLOCK_SIZE)
    {
        Block* new_block = (Block*)((char*)(block + 1) + size);
        new_block->size = block->size - size - sizeof(Block);
        new_block->free = 1;
        new_block->is_mmap = 0;
        new_block->next = block->next;
        new_block->prev = block;
        if (block->next != NULL)
            block->next->prev = new_block;
        block->next = new_block;
        block->size = size;
    }
};

void coalesce_block(Block* block)
{
    if (block->next && block->next->free)
    {
        block->size += block->next->size + sizeof(Block);
        block->next = block->next->next;
        if (block->next)
            block->next->prev = block;
    }
    if (block->prev && block->prev->free)
    {
        coalesce_block(block->prev);
    }
};

void free_block(Block* block)
{
    if (!block)
        return;
    block->free = 1;
    coalesce_block(block);
};