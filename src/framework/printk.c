#include "printk.h"

void printChar(char str, int cursor, int color)
{
	static struct VgaChar *vga_buf = (struct VgaChar *)VGA_MEMORY;
	vga_buf[cursor].character = str;
	vga_buf[cursor].color = color;
}

void puts(char *str, int *cursor)
{
	while (*str != '\0') {
		if (*cursor >= MAX_SCREEN_SIZE)
			*cursor = 0;
		printChar(*str, *cursor, GREY);
		str++;
		(*cursor)++;
	}
}

void formatEvaluation(int *argp, int *cursor, char ch)
{
	switch ((enum FormatSpecifiers)ch) {
	case SIGNED_DECIMAL:
		puts(intToStr(*argp, 10), cursor);
		break;
	case UNSIGNED_DECIMAL:
		puts(uintToStr(*argp, 10), cursor);
		break;
	case UNSIGNED_OCTAL:
		puts(uintToStr(*argp, 8), cursor);
		break;
	case UNSIGNED_HEX:
		puts(uintToStr(*argp, 16), cursor);
		break;
	case CHARACTER:
		printChar(*(char *)argp, *cursor, GREY);
		break;
	case STRING:
		puts((char *)*argp, cursor);
		break;
	case POINTER_ADDRESS:
		printk("0x%x", (int)*argp);
		break;
	case MODULO:
		static char mod = '%';
		printChar(mod, *cursor, GREY);
		break;
	}
}

void printk(char *str, ...)
{
	int *argp = (int *)&str;
	argp += sizeof(str) / sizeof(int);

	static int cursor = 0;
	while (*str != '\0') {
		if (cursor >= MAX_SCREEN_SIZE) {
			cursor = 0;
		}
		switch (*str) {
		case (MODULO):
			char nextChar = *(str + 1);
			if (nextChar == '\0') {
				str++;
				continue;
			}
			formatEvaluation(argp, &cursor, nextChar);
			argp++;
			str += 2;
			break;
		case (LF):
			cursor += SCREEN_WIDTH - (cursor % SCREEN_WIDTH);
			str++;
			break;
		default:
			printChar(*str, cursor, GREY);
			str++;
			cursor++;
		}
	}
}
