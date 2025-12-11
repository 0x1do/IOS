#pragma once
#include "kernel.h"

void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t n);
/*
 *   todo: Implement memmove, memcmp, malloc/free, calloc, realloc,
 */
