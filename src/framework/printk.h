#pragma once
#define VGA_MEMORY 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define MAX_SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#include "utils.h"

enum Colors {
	BLACK = 0X0,
	GREEN = 0X2,
	RED = 0X4,
	ORANGE = 0X6,
	GREY = 0X7,
	YELLOW = 0XE
};

struct VgaChar {
	char character;
	char color;
};

enum FormatSpecifiers {
	SIGNED_DECIMAL = 'd',
	UNSIGNED_DECIMAL = 'u',
	UNSIGNED_OCTAL = 'o',
	UNSIGNED_HEX = 'x',
	CHARACTER = 'c',
	STRING = 's',
	POINTER_ADDRESS = 'p',
	MODULO = '%',
	LF = '\n'
};

void printChar(char str, int cursor, int color);
void puts(char *str, int *cursor);
enum FormatSpecifiers getFormatSpecifier(char specifier);
void formatEvaluation(int *argp, int *cursor, char ch);
void printk(char *str, ...);
