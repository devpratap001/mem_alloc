#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "../include/mem_alloc_internals.h"

void *mmap_alloc(size_t size)
{
    size_t tot_size = sizeof(Block) + size;
    Block *ptr = mmap(NULL, tot_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED)
    {
        return NULL;
    }
    ptr->free = 0;
    ptr->next = NULL;
    ptr->prev = NULL;
    ptr->is_mmap = 1;
    ptr->size = size;
    return (void *)(ptr + 1);
};

void mmap_free(Block *block)
{
    if (munmap((Block *)block, block->size + sizeof(Block)))
    {
        perror("Can't free memory");
        exit(EXIT_FAILURE);
    }
    return;
};