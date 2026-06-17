#include "string.h"

char *uintToStr(uint32_t value, int base, char *return_buffer)
{
	char len = 32;
	char buf[len];
	return_buffer[0] = '\0';
	int i;

	if (base < 2 || base > 16) {
		return_buffer[0] = '\0';
		return return_buffer;
	}

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
	uintToStr(0u - (uint32_t)value, base, return_buffer + 1);
	return return_buffer;
}

char *ulongToStr(uint64_t value, int base, char *return_buffer)
{
	char len = 64;
	char buf[len];
	int i = 0;

	if (base < 2 || base > 16) {
		return_buffer[0] = '\0';
		return return_buffer;
	}

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
		ulongToStr(0ull - (uint64_t)value, base, return_buffer + 1);
		return return_buffer;
	}
}

int strcmp(const char *str1, const char *str2)
{
	while (*str1 != '\0' && *str2 != '\0') {
		if (*str1 != *str2) {
			return (unsigned char)(*str1) - (unsigned char)(*str2);
		}
		str1++;
		str2++;
	}

	return (unsigned char)(*str1) - (unsigned char)(*str2);
}

char *strcpy(char *dest, const char *src)
{
	char *retptr = dest;
	while (*src != '\0') {
		*dest = *src;
		dest++;
		src++;
	}
	*dest = '\0';
	return retptr;
}

char *strcat(char *dest, const char *src)
{
	char *retptr = dest;
	while (*dest != '\0') {
		dest++;
	}

	while (*src != '\0') {
		*dest = *src;
		dest++;
		src++;
	}

	*dest = '\0';
	return retptr;
}

char *strchr(const char *str, char ch)
{
	while (*str != '\0') {
		if (*str == ch) {
			return (char *)str;
		}
		str++;
	}

	if (ch == '\0')
		return (char *)str;

	return NULL;
}

size_t strlen(const char *str)
{
	size_t length = 0;

	while (*str != '\0') {
		length++;
		str++;
	}

	return length;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	if (dest == NULL || src == NULL) {
		return NULL;
	}

	size_t i;
	for (i = 0; i < n && src[i] != '\0'; i++) {
		dest[i] = src[i];
	}

	for (; i < n; i++) {
		dest[i] = '\0';
	}

	return dest;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	const unsigned char *u1 = (const unsigned char *)s1;
	const unsigned char *u2 = (const unsigned char *)s2;

	while (n-- > 0) {
		if (*u1 != *u2) {
			return (*u1 > *u2) ? 1 : -1;
		}
		if (*u1 == '\0') {
			return 0;
		}
		u1++;
		u2++;
	}
	return 0;
}

int toupper(int c)
{
	if (c >= 'a' && c <= 'z') {
		return c - ('a' - 'A');
	}
	return c;
}

int isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

int isalpha(int c)
{
	return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

int atoi(const char *str)
{
	int result = 0;
	int sign = 1;

	while (isspace(*str))
		str++;

	if (*str == '-') {
		sign = -1;
		str++;
	} else if (*str == '+') {
		str++;
	}

	while (isdigit(*str)) {
		result = result * 10 + (*str - '0');
		str++;
	}

	return sign * result;
}
