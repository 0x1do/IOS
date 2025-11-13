#pragma once
#define MULTIBOOT_MAGIC 0x1BADB002
#define MULTIBOOT_CHECKSUM -(0x1BADB002)
typedef enum { false = 0, true = 1 } bool;
#include "framework/printk.h"
#include "gdt.h"

void kernel_main(void);
