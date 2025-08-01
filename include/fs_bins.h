#ifndef FS_BINS_H
#define FS_BINS_H

#include <stddef.h>
#include "../include/mem_alloc_internals.h"

// no of bin size categories
#define MAX_BIN_CATEGORIES 7    // 8, 16, 32, 34, 128, 256, 512

// bins array
extern Block *bins[MAX_BIN_CATEGORIES];

// get the corresponding bin category, chunk falls in
int get_bin_index(size_t);

// find the free block in a particular bin
Block* find_fit_bin(Block*, size_t);

// pop from freelist
Block* pop_bins(Block*, size_t);

// push block to freelist
Block* push_bins(Block*, size_t);

#endif