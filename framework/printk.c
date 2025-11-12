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
	switch ((enum FormatSpecifiers)ch) {
	case SIGNED_DECIMAL:
		printk(int_to_str(*argp, 10));
		break;
	case UNSIGNED_DECIMAL:
		printk(uint_to_str(*argp, 10));
		break;
	case UNSIGNED_OCTAL:
		printk(uint_to_str(*argp, 8));
		break;
	case UNSIGNED_HEX:
		printk(uint_to_str(*argp, 16));
		break;
	case CHARACTER:
		setChar((char *)argp, *offset, GREY);
		break;
	case STRING:
		printk((char *)*argp);
		break;
	case POINTER_ADDRESS:
		printk("0x%x", (int)*argp);
		break;
	case MODULO:
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
			char nextChar = *(str + 1);
			if(nextChar == '\0')
			{
				str++;
				continue;
			}
			formatEvaluation(argp, &offset, nextChar);
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
