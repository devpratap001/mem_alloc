#include <stdio.h>
#include "../include/fs_bins.h"

int bin_size[MAX_BIN_CATEGORIES] = {8, 16, 32, 64, 128, 256, 512};
Block* bins[MAX_BIN_CATEGORIES] = {0};

int get_bin_index(size_t size)
{
    for (int i = 0; i < MAX_BIN_CATEGORIES; i++)
    {
        if (size <= bin_size[i])
            return i;
    };
    return -1;
};

Block* find_fit_bin(Block* block, size_t size)
{
    Block* temp = block;
    while (temp)
    {
        if (temp->size >= size)
            return temp;
        temp = temp->next;
    };
    return NULL;
};

Block* pop_bins(Block* block, size_t size)
{
    if (get_bin_index(ALIGN(size)) == -1)
        return NULL;
    block->free = 0;
    if (block->next)
    {
        block->next->prev = block->prev;
    }
    if (block->prev)
    {
        block->prev->next = block->next;
    }
    else
    {
        bins[get_bin_index(ALIGN(size))] = block->next;
    }
    block->prev = NULL;
    block->next = NULL;
    return block;
};

Block* push_bins(Block* block, size_t size)
{
    size_t index = get_bin_index(ALIGN(size));
    if (index == -1)
        return NULL;
    block->free = 1;
    Block* temp = bins[index];
    block->next = temp;
    if (temp)
        temp->prev = block;
    bins[index] = block;
    return block;
};