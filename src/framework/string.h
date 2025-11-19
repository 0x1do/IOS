#pragma once
#define DIGITS "0123456789abcdef"

char *uintToStr(unsigned int value, int base, char *return_buffer);
char *intToStr(int value, int base, char *return_buffer);
char *ulongToStr(unsigned long value, int base, char *return_buffer);
char *longToStr(long value, int base, char *return_buffer);
