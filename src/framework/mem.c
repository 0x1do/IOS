#include "mem.h"

void *memcpy(void *dest, const void *src, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		((char *)dest)[i] = ((const char *)src)[i];
	}
	return dest;
}

void *memset(void *ptr, int value, size_t num)
{
	unsigned char *p = ptr;
	while (num--) {
		*p++ = (unsigned char)value;
	}
	return ptr;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
	const unsigned char *p1 = (unsigned char *)ptr1;
	const unsigned char *p2 = (unsigned char *)ptr2;

	while (num > 0) {
		if (*p1 != *p2) {
			return (*p1 > *p2) ? 1 : -1;
		}
		p1++;
		p2++;
		num--;
	}

	return 0;
}
