# Custom Memory Allocator

A custom memory allocator similar to `malloc` and `free` build in C language. It provides several functions to allocate dynamic heap memory during program runtime and efficiently free allocated memory.

It manages memory directly from OS using `sbrk` and `mmap` and provides efficient allocation through segmented freelists, with block splitting and coalescing.

## Features

1. Segregated Free lists
2. Block splitting and Coalescing
3. mmap support for large allocations
4. Support for malloc, calloc, realloc and free
5. Alignment guarantee and double free detection.

## Project Structure

```
./
    |- build/
        |- deps/                    // header dependencies
        |- objs/                    // object files
        |- mem_alloc                // compiled bianry output file

    |- include/
        |- fs_bins.h                // header for fast and small bins
        |- mem_alloc_internals.h    // mem_alloc internal implementation
        |- mem_alloc.h              // public API

    |- src/
        |- block_alloc.c            // fallback freelist and memory allocation
        |- fs_bins.c                // segregated bins
        |- mem_alloc.c              // core public API implementation
        |- mmap_alloc.c             // mmap allocations

    |- .gitignore
    |- Makefile
    |- README.md
```

## Design

- **Bins:** Segregated by block sizes(e.g., 8, 16, 32, 64bytes, ...)
- **First Fit:** For selecting free bins of particular category
- **Heap Growth** via `sbrk` with page-sized increment and unused space tracking.
- **Large allocations** handled using `mmap`.

## Memory block layout

```
[Header(metadata) | payload | Footer(size)]
```

## Usage

### Build
```
make all
```

### Testing

create `index.c` in src folder to test for various testcases through functions via `mem_alloc.h` header.

## Future Improvements

- Best fit allocation strategy for segregated bins.
- delayed coalescing and with bins.
- Thread safety and per thread arenas/tcache.
- Safety and security(overflow, underflow, metadata hashing, etc.)