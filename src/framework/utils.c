#include "utils.h"

char *uintToStr(unsigned int value, int base)
{
	char buf[32];
	static char return_buffer[33];
	return_buffer[0] = '\0';
	int i = 0;

	if (value == 0) {
		return_buffer[0] = '0';
		return_buffer[1] = '\0';
		return return_buffer;
	}

	while (value > 0 && i < 32) {
		buf[i] = DIGITS[value % base];
		value /= base;
		i++;
	}

	for (int j = i - 1; j >= 0; j--) {
		return_buffer[i - j - 1] = buf[j];
	}

	return_buffer[i] = '\0';

	return return_buffer;
}

char *intToStr(int value, int base)
{
	if (value >= 0) {
		return uintToStr((unsigned int)value, base);
	}
	value = -value;
	char *buf = uintToStr(value, base);
	if (!buf)
		return 0;

	static char *negative_buffer;
	*negative_buffer = '-';
	*(negative_buffer + 1) = '\0';
	int i;
	for (i = 0; buf[i] != '\0'; i++) {
		*(negative_buffer + 1 + i) = buf[i];
	}
	*(negative_buffer + 1 + i) = '\0';
	return negative_buffer;
}

