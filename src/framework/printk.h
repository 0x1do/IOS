#pragma once
#include "flanterm.h"
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

enum CountBase { BINARY = 0, OCTAL = 8, DECIMAL = 10, HEX = 16 };

void putChar(char ch);
void putS(char *str);
enum FormatSpecifiers getFormatSpecifier(char specifier);
void longEvaluation(va_list *args, const char *str);
void formatEvaluation(va_list *args, const char *str);
void printk(const char *str, ...);
