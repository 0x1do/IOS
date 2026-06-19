#include "crypto.h"

void cryptXor(uint8_t *buf, int len, const char *password)
{
	int klen = 0;
	int i;

	while (password[klen])
		klen++;
	if (klen == 0)
		return;

	for (i = 0; i < len; i++)
		buf[i] ^= (uint8_t)password[i % klen] ^ (uint8_t)(i * 31);
}
