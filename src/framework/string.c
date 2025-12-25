#include "string.h"

char *uintToStr(uint32_t value, int base, char *return_buffer)
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
		return uintToStr((uint32_t)value, base, return_buffer);
	}
	return_buffer[0] = '-';
	uintToStr((uint32_t)-value, base, return_buffer + 1);
	return return_buffer;
}

char *ulongToStr(uint64_t value, int base, char *return_buffer)
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
		return ulongToStr((uint64_t)value, base, return_buffer);
	} else {
		return_buffer[0] = '-';
		ulongToStr((uint64_t)(-value), base, return_buffer + 1);
		return return_buffer;
	}
}

int strcmp(const char *str1, const char *str2) {
    while (*str1 != '\0' && *str2 != '\0') {
        if (*str1 != *str2) {
            return (unsigned char)(*str1) - (unsigned char)(*str2);
        }
        str1++;
        str2++;
    }

    return (unsigned char)(*str1) - (unsigned char)(*str2);
}

/*
 *	TODO: implement strlen, strcpy, strcat, strstr, strchr
 */
