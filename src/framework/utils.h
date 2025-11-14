#pragma once
#include "../kernel.h"
#define DIGITS "0123456789abcdef"

char *uintToStr(unsigned int value, int base);
char *intToStr(int value, int base);