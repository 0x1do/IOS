#pragma once
#define MULTIBOOT_MAGIC 0x1BADB002
#define MULTIBOOT_CHECKSUM -(0x1BADB002)
#include "../framework/printk.h"
typedef enum { false = 0, true = 1 } bool;

void kernel_main(void);
