#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/fs_bins.h"

static Block *head = NULL;
static int sbrk_calls = 0;
void *heap_start = NULL, *heap_end = NULL;

Block *find_fit(size_t size)
{
    Block *curr = head;
    while (curr)
    {
        if ((curr->free & CURR_FREE) && curr->size >= size)
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
    if (!heap_start)
        heap_start = (void *)block;
    heap_end = (void *)block + chunk_size;
    sbrk_calls++;
    block->size = ALIGN(size);
    block->free = 0;
    block->next = NULL;
    block->prev = NULL;
    block->is_mmap = 0;

    size_t leftover_size = chunk_size - total_size;
    if (leftover_size >= sizeof(Block) + MIN_BLOCK_SIZE)
    {
        leftover = (Block *)((char *)block + total_size);
        leftover->free = 0;
        leftover->free |= CURR_FREE;
        leftover->is_mmap = 0;
        leftover->next = NULL;
        leftover->prev = NULL;
        leftover->size = leftover_size - sizeof(Block);
        Block *result = push_bins(leftover, leftover->size);
        if (!result)
        {
            leftover->next = head;
            if (head)
                head->prev = leftover;
            head = leftover;
        }
    }
    set_prev_full(block);

    return block;
};

void remove_from_freelist(Block *block)
{
    set_prev_full(block);
    if (block->next)
        block->next->prev = block->prev;
    if (block->prev)
        block->prev->next = block->next;
    else
        head = block->next;
    block->prev = NULL;
    block->next = NULL;
};

void split_block(Block *block, size_t size)
{
    if (block->size >= ALIGN(size + sizeof(Blockfooter)) + MIN_BLOCK_SIZE)
    {
        Block *new_block = (Block *)((char *)(block + 1) + ALIGN(size + sizeof(Blockfooter)));
        new_block->size = block->size - ALIGN(size + sizeof(Blockfooter)) - sizeof(Block);
        new_block->free = 0;
        new_block->free |= CURR_FREE;
        new_block->is_mmap = 0;
        new_block->next = block->next;
        if (block->next != NULL)
            block->next->prev = new_block;
        new_block->prev = block;
        block->next = new_block;
        block->size = ALIGN(sizeof(Blockfooter) + size);
        Blockfooter* n_footer = (void*)(new_block + 1) + new_block->size - sizeof(Blockfooter);
        n_footer->size = new_block->size;
        set_prev_free(new_block);
    }
};

Block *coalesce_block(Block *block)
{
    Block *next = (void *)(block + 1) + block->size;
    if ((void *)next < heap_end)
    {
        if (next->free & CURR_FREE)
        {
            remove_from_freelist(next);
            block->size += sizeof(Block) + next->size;
            Blockfooter *new_footer = (void *)(block + 1) + block->size - sizeof(Blockfooter);
            new_footer->size = block->size;
            set_prev_free(block);
        };
    }

    if ((block->free & PREV_FREE) == 0)
        return block;
    Blockfooter *prev_footer = (void *)block - sizeof(Blockfooter);
    Block *prev = (void *)block - prev_footer->size - sizeof(Block);
    if ((void*)prev < heap_start)
        return block;
    if (prev->free & CURR_FREE)
    {
        remove_from_freelist(block);
        prev->size += sizeof(Block) + block->size;
        Blockfooter *prev_footer = (void *)(prev + 1) + prev->size - sizeof(Blockfooter);
        prev_footer->size = prev->size;
        set_prev_free(prev);
    };
    return prev;
};

void free_block(Block *block)
{
    if (!block)
        return;
    block->free |= CURR_FREE;
    block->next = head;
    if (head)
        head->prev = block;
    head = block;
    block = coalesce_block(block);
};

void set_prev_free(Block* block)
{
    Block* phy_next = (void*)(block + 1) + block->size;
    if ((void*)phy_next < heap_end)
    {
        phy_next->free |= PREV_FREE;
    }
};

void set_prev_full(Block* block)
{
    Block* phy_next = (void*)(block + 1) + block->size;
    if ((void*)phy_next < heap_end)
    {
        phy_next->free &= ~(PREV_FREE);
    }
};

void print_heap_status(void)
{
    printf("total allocations: %d; hs: %p; he: %p\n", sbrk_calls, heap_start, heap_end);
    Block *temp;
    for (int i = 0; i < MAX_BIN_CATEGORIES; i++)
    {
        temp = bins[i];
        printf("%d: ", i);
        while (temp != NULL)
        {
            printf("[block(%zu)(%d) - %p] ", temp->size, temp->free, (char *)(temp + 1));
            temp = temp->next;
        }
        printf("\n");
    }
    temp = head;
    while (temp != NULL)
    {
        printf("[block(%zu)(%d) - %p] ", temp->size, temp->free, (char *)(temp + 1));
        temp = temp->next;
    }
    printf("\n=============================\n");
};