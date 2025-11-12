#pragma once
#define VGA_MEMORY 0xB8000
#define DIGITS "0123456789abcdef"
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define MAX_SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define GREY 0x07

struct vga_char {
	char character;
	char attribute;
};

enum FormatSpecifiers {
	SIGNED_DECIMAL = 'd',
	UNSIGNED_DECIMAL = 'u',
	UNSIGNED_OCTAL = 'o',
	UNSIGNED_HEX = 'x',
	CHARACTER = 'c',
	STRING = 's',
	POINTER_ADDRESS = 'p',
	MODULO = '%'
};

void setChar(char *str, int offset, int color);
enum FormatSpecifiers getFormatSpecifier(char specifier);
char *uint_to_str(unsigned int value, int base);
char *int_to_str(int value, int base);
void formatEvaluation(int *argp, int *offset, char ch);
void printk(char *str, ...);
