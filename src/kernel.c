#include "kernel.h"

__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = { MULTIBOOT_MAGIC,
										  0x0, // flags
										  MULTIBOOT_CHECKSUM };

void kernel_main(void)
{
	while (1) {
		__asm__("hlt");
	}
}
