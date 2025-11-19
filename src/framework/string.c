#include "string.h"

char *uintToStr(unsigned int value, int base, char *return_buffer)
{
	char len = 32;
	char buf[len];
	return_buffer[0] = '\0';
	int i;

	if (value == 0) {
		return_buffer[0] = '0';
		return_buffer[1] = '\0';
		return return_buffer;
	}

	for (i = 0; value > 0 && i < len; i++) {
		buf[i] = DIGITS[value % base];
		value /= base;
	}

	for (int j = i - 1; j >= 0; j--) {
		return_buffer[i - j - 1] = buf[j];
	}

	return_buffer[i] = '\0';
	return return_buffer;
}

char *intToStr(int value, int base, char *return_buffer)
{
	if (value >= 0) {
		return uintToStr((unsigned int)value, base, return_buffer);
	}
	return_buffer[0] = '-';
	uintToStr((unsigned int)-value, base, return_buffer + 1);
	return return_buffer;
}

char *ulongToStr(unsigned long value, int base, char *return_buffer)
{
	char len = 64;
	char buf[len];
	int i = 0;
	if (value == 0) {
		return_buffer[0] = '0';
		return_buffer[1] = '\0';
		return return_buffer;
	}

	while (value > 0 && i < len) {
		buf[i++] = DIGITS[value % base];
		value /= base;
	}

	for (int j = 0; j < i; j++)
		return_buffer[j] = buf[i - j - 1];

	return_buffer[i] = '\0';
	return return_buffer;
}

char *longToStr(long value, int base, char *return_buffer)
{
	if (value >= 0) {
		return ulongToStr((unsigned long)value, base, return_buffer);
	} else {
		return_buffer[0] = '-';
		ulongToStr((unsigned long)(-value), base, return_buffer + 1);
		return return_buffer;
	}
}

/*
 *	todo: implement strlen, strcmp, strcpy, strcat, strstr, strchr
 */
