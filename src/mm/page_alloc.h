#pragma once
#include "kernel.h"

#define PHYSICAL_MEMORY_SIZE (1024 * 1024 * 30) /* 30 mb for now */
#define PAGE_SIZE 4096
#define BITMAP_SIZE (PHYSICAL_MEMORY_SIZE / PAGE_SIZE / 8) /* in bytes */
#define FREE 0
#define ALLOCATED 1

extern const uint64_t endkernel;
extern uint8_t bitmap[BITMAP_SIZE];
extern uint16_t page_counts[];

void initAllocator();
void *kalloc(size_t size);
void *krealloc(void *addr, uint64_t new_size);
void page_free(void *addr);
