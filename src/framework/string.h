#pragma once
#define DIGITS "0123456789abcdef"
#include "kernel.h"

#define isspace(c) ((c) == ' '  || (c) == '\t' || (c) == '\n' || \
                    (c) == '\v' || (c) == '\f' || (c) == '\r')

char *uintToStr(uint32_t value, int base, char *return_buffer);
char *intToStr(int value, int base, char *return_buffer);
char *ulongToStr(uint64_t value, int base, char *return_buffer);
char *longToStr(long value, int base, char *return_buffer);
int strcmp(const char *str1, const char *str2);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
char *strchr(const char *str, char ch);
size_t strlen(const char *str);
char *strncpy(char *dest, const char *src, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
int toupper(int c);
int isdigit(int c);
int isalpha(int c);
int atoi(const char *str);
