#pragma once
#include "kernel.h"

void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t n);
void write_to_port(uint16_t port, uint32_t data);
/*
 *   todo: Implement memmove, memcmp, malloc/free, calloc, realloc,
 */
