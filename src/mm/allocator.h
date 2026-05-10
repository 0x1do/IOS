#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "kernel.h"
#include <stdbool.h>
#include "mem.h"

#define PAGE_ALIGN(x) ((((x) + 4095) / 4096) * 4096)
#define ALLOC_MAGIC 0xCACECAFE
#define KMALLOC_MIN  32
#define ALLOC_ALIGNMENT 16
#define KMALLOC_MAX 4096


void* page_alloc(size_t pages);
void  page_dealloc(void* data, size_t pages);

void* kernel_malloc(size_t memory);
void  kernel_free(void* data, size_t size);

struct list {
    struct list *next, *prev;
};

typedef struct {
    struct list full, partial, free;
    size_t totalobjs;
    size_t inuse;
    size_t objsize;
    size_t objs_per_slab;
} Cache;

extern Cache* cache_cache;


int allocator_init(void);
intptr_t cache_grow(Cache* cache);
void*    cache_alloc(Cache* cache);
void     cache_dealloc(Cache* cache, void* p);

void     init_cache(Cache* c, size_t objsize);
Cache*   create_new_cache(size_t objsize);

void *kmalloc(size_t size);
void  kfree(void *ptr);

#endif
