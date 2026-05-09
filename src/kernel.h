#pragma once
#define NULL ((void *)0)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#include <stddef.h>
#include <stdint.h>
void kernelMain(void);

__attribute__((noreturn)) void _exit(int status);
