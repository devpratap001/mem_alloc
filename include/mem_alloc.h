#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

//allocate dynamic memory on heap segment
void* mem_alloc(size_t);

//allocate and initialize dynamic memory to zero using calloc
void* mem_calloc(int, size_t);

//reallocate previously allocated memory
void* mem_realloc(void*, size_t);

//free dynamic memory allocated using mem_alloc
void free_alloc(void*);

// print heap layout
void print_heap(void);

#endif