#pragma once
#define DIGITS "0123456789abcdef"
#include "kernel.h"

char *uintToStr(uint32_t value, int base, char *return_buffer);
char *intToStr(int value, int base, char *return_buffer);
char *ulongToStr(uint64_t value, int base, char *return_buffer);
char *longToStr(long value, int base, char *return_buffer);
int strcmp(const char *str1, const char *str2);
