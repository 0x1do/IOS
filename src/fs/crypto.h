#pragma once
#include <stdint.h>

/* check byte prepended to plaintext; detects a wrong password on decrypt */
#define CRYPT_CHECK_BYTE 0xA5

/* ext2 EXT2_ENCRYPT_FL; marks a file encrypted in Inode.flags */
#define FS_ENCRYPT_FL 0x800

void cryptXor(uint8_t *buf, int len, const char *password);
