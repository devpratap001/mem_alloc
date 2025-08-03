#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/mem_alloc.h"
#include "../include/fs_bins.h"

void *mem_alloc(size_t size)
{
    if (size <= 0)
        return NULL;
    if (size > MMAP_THRESHOLD)
    {
        return mmap_alloc(ALIGN(size));
    }
    size += sizeof(Blockfooter);
    Block *block = NULL;
    if (get_bin_index(ALIGN(size)) >= 0)
    {
        size_t index = get_bin_index(ALIGN(size));
        while (index < MAX_BIN_CATEGORIES)
        {
            block = find_fit_bin(bins[index], ALIGN(size));
            if (block)
                break;
            index++;
        }
    };
    if (block)
    {
        set_prev_full(block);
        block = pop_bins(block, block->size);
        return (void *)(block + 1);
    };
    block = find_fit(ALIGN(size));
    if (!block)
    {
        block = allocate_block(ALIGN(size));
        if (!block)
            return NULL;
    }
    else
    {
        split_block(block, ALIGN(size));
        remove_from_freelist(block);
        block->free &= ~(CURR_FREE);
    }

    return (void *)(block + 1);
};

void *mem_calloc(int num, size_t size)
{
    if (size <= 0 || num <= 0)
        return NULL;
    size_t total_size = num * size;
    void *ptr = mem_alloc(total_size);
    if (!ptr)
        return NULL;
    memset(ptr, 0, total_size);
    return ptr;
};

void *mem_realloc(void *ptr, size_t size)
{
    if (!ptr)
        return mem_alloc(size);
    if (size <= 0)
    {
        free_alloc(ptr);
        return NULL;
    }
    Block *block = (Block *)ptr - 1;
    if (block->size >= size)
    {
        if (block->size >= ALIGN(size + sizeof(Blockfooter)) + MIN_BLOCK_SIZE)
        {
            split_block(block, size);
            block->next = NULL;
            block->prev = NULL;
            Block* next_block = (void*)(block + 1) + block->size;
            int index = get_bin_index(next_block->size);
            if (index >= 0)
            {
                push_bins(next_block, next_block->size);
                return ptr;
            }
            free_block(next_block);
            return ptr;
        }
    }
    else
    {
        Block *phy_next = (void *)(block + 1) + block->size;
        if ((void*)phy_next < heap_end)
        {
            if ((phy_next->free & CURR_FREE) && (block->size + phy_next->size + sizeof(Block) >= size))
            {
                int index = get_bin_index(ALIGN(phy_next->size));
                if (index >= 0)
                {
                    set_prev_full(phy_next);
                    phy_next = pop_bins(phy_next, phy_next->size);
                    phy_next->free &= ~(CURR_FREE);
                    block->size += sizeof(Block) + phy_next->size;
                    Blockfooter *b_footer = (void *)(block + 1) + block->size - sizeof(Blockfooter);
                    b_footer->size = block->size;
                    return (void*)(block + 1);
                }
                remove_from_freelist(phy_next);
                phy_next->free &= ~(CURR_FREE);
                block->size += sizeof(Block) + phy_next->size;
                Blockfooter *b_footer = (void *)(block + 1) + block->size - sizeof(Blockfooter);
                b_footer->size = block->size;
                return (void*)(block + 1);
            }
        }
    }
    void *new_ptr = mem_alloc(size);
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
    Block *block = (Block *)memptr - 1;
    if (block->free & CURR_FREE)
    {
        fprintf(stderr, "Double free detected: Can't perform this action!\n");
        exit(EXIT_FAILURE);
    }
    if (block->is_mmap)
    {
        mmap_free(block);
        return;
    }
    Blockfooter *footer = (void *)(block + 1) + block->size - sizeof(Blockfooter);
    footer->size = block->size;
    set_prev_free(block);
    size_t index = get_bin_index(block->size);
    if (index >= 0 && index < MAX_BIN_CATEGORIES)
    {
        push_bins(block, block->size);
        return;
    }
    free_block(block);
};

void print_heap(void)
{
    print_heap_status();
};