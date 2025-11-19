#pragma once
#define CGA_MEMORY 0xB8000
#define CGA_MEMORY_ODD (0xB8000 + 0x2000)
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define MAX_SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#include "string.h"

typedef __builtin_va_list va_list;
#define va_start __builtin_va_start
#define va_arg __builtin_va_arg
#define va_end __builtin_va_end

enum FormatSpecifiers {
	SIGNED_DECIMAL = 'd',
	UNSIGNED_DECIMAL = 'u',
	UNSIGNED_OCTAL = 'o',
	UNSIGNED_HEX = 'x',
	CHARACTER = 'c',
	STRING = 's',
	POINTER_ADDRESS = 'p',
	MODULO = '%',
	LONG = 'l'
};

void printChar(char str);
void puts(char *str);
enum FormatSpecifiers getFormatSpecifier(char specifier);
void formatEvaluation(va_list *args, char *str);
void printk(char *str, ...);
