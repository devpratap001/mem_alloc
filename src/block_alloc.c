#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/fs_bins.h"

static Block *head = NULL;
static int sbrk_calls = 0;

Block *find_fit(size_t size)
{
    Block *curr = head;
    while (curr)
    {
        if (curr->free && curr->size >= size)
        {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
};

Block *allocate_block(size_t size)
{
    size_t total_size = sizeof(Block) + ALIGN(size);
    size_t chunk_size = ALIGN_UP(total_size, PAGE_SIZE);
    Block *block = NULL, *leftover = NULL;

    if ((block = sbrk(chunk_size)) == (void *)-1)
        return NULL;
    sbrk_calls++;
    block->size = ALIGN(size);
    block->free = 0;
    block->next = NULL;
    block->prev = NULL;
    block->is_mmap = 0;

    size_t leftover_size = chunk_size - total_size;
    if (leftover_size >= sizeof(Block) + MIN_BLOCK_SIZE)
    {
        leftover = (Block*)((char*)block + total_size);
        leftover->free = 1;
        leftover->is_mmap = 0;
        leftover->next = NULL;
        leftover->prev = NULL;
        leftover->size = leftover_size - sizeof(Block);
        Block* result = push_bins(leftover, leftover->size);
        if (!result)
        {
            leftover->next = head;
            if (head)
                head->prev = leftover;
            head = leftover;
        }
    }

    return block;
};

void remove_from_freelist(Block* block)
{
    if (block->next)
        block->next->prev = block->prev;
    if (block->prev)
        block->prev->next = block->next;
    else
        head = block->next;
    block->prev = NULL;
    block->next= NULL;
};

void split_block(Block *block, size_t size)
{
    if (block->size >= ALIGN(size) + sizeof(Block) + MIN_BLOCK_SIZE)
    {
        Block *new_block = (Block *)((char *)(block + 1) + ALIGN(size));
        new_block->size = block->size - ALIGN(size) - sizeof(Block);
        new_block->free = 1;
        new_block->is_mmap = 0;
        new_block->next = block->next;
        if (block->next != NULL)
            block->next->prev = new_block;
        new_block->prev = block;
        block->next = new_block;
        block->size = size;
    }
};

void coalesce_block(Block *block)
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

void free_block(Block *block)
{
    if (!block)
        return;
    block->free = 1;
    // coalesce_block(block);
};

void print_heap_status(void)
{
    printf("total allocations: %d\n", sbrk_calls);
    Block* temp;
    for (int i = 0; i < MAX_BIN_CATEGORIES; i++)
    {
        temp = bins[i];
        printf("%d: ", i);
        while (temp != NULL)
        {
            printf("[block(%zu)(%d) - %p] ", temp->size, temp->free, (char*)(temp + 1));
            temp = temp->next;
        }
        printf("\n");
    }
    temp = head;
    while (temp != NULL)
    {
        printf("[block(%zu)(%d) - %p] ", temp->size, temp->free, (char*)(temp + 1));
        temp = temp->next;
    }
    printf("\n=============================\n");
};