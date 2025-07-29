#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/mem_alloc_internals.h"

static Block *head = NULL, *tail = NULL;

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