#include "printk.h"

void setChar(char *str, int offset, int color)
{
	static struct vga_char *vga_buf = (struct vga_char *)VGA_MEMORY;
	vga_buf[offset].character = *str;
	vga_buf[offset].attribute = color;
}

char *uint_to_str(unsigned int value, int base)
{
	char buf[32];
	static char retbuf[33];
	retbuf[0] = '\0';
	int i = 0;

	if (value == 0) {
		retbuf[0] = '0';
		retbuf[1] = '\0';
	}

	while (value > 0 && i < 32) {
		buf[i] = DIGITS[value % base];
		value /= base;
		i++;
	}

	for (int j = i - 1; j >= 0; j--) {
		retbuf[i - j - 1] = buf[j];
	}

	retbuf[i] = '\0';
	return retbuf;
}

char *int_to_str(int value, int base)
{
	if (value >= 0) {
		return uint_to_str(value, base);
	}
	value = -value;
	char *buf = uint_to_str(value, base);
	if (!buf)
		return 0;

	static char *negbuf;
	*negbuf = '-';
	*(negbuf + 1) = '\0';
	int i;
	for (i = 0; buf[i] != '\0'; i++) {
		*(negbuf + 1 + i) = buf[i];
	}
	*(negbuf + 1 + i) = '\0';
	return negbuf;
}

void formatEvaluation(int *argp, int *offset, char ch)
{
	switch (ch) {
	case 'd': // Signed decimal integer
		printk(int_to_str(*argp, 10));
		break;
	case 'u': // Unsigned decimal integer
		printk(uint_to_str(*argp, 10));
		break;
	case 'o': // Unsigned octal
		printk(uint_to_str(*argp, 8));
		break;
	case 'x': // Unsigned hexadecimal integer
		printk(uint_to_str(*argp, 16));
		break;
	case 'c': // Character
		setChar((char *)argp, *offset, GREY);
		break;
	case 's': // String of characters
		printk((char *)*argp);
		break;
	case 'p': // Pointer address
		printk("0x%x", (int)*argp);
		break;
	case '%':
		static char mod = '%';
		setChar(&mod, *offset, GREY);
		break;
	}
}

void printk(char *str, ...)
{
	int *argp = (int *)&str;
	argp += sizeof(str) / sizeof(int); // second arg

	static int offset = 0;
	while (*str != '\0') {
		if (offset >= MAX_SCREEN_SIZE) {
			offset = 0;
		}
		if (*str == '%') {
			formatEvaluation(argp, &offset, *(str + 1));
			argp++;
			str += 2;
			continue;
		}
		if (*str == '\n') {
			offset += SCREEN_WIDTH - (offset % SCREEN_WIDTH);
            setChar(str, offset, GREY);
		    str++;
            continue;
		}
        setChar(str, offset, GREY);
		str++;
		offset++;
	}
}
