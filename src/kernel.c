#include "kernel.h"

__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = { MULTIBOOT_MAGIC,
										  0x0, // flags
										  -(MULTIBOOT_MAGIC) };

void kernelMain(void)
{
	initGDT();
	printk("GDTR base = %p, limit = %d\n", gdtr.base_address, gdtr.limit);
	printDescriptorEntry(gdt_table.table[1]);
	while (1) {
		__asm__("hlt");
	}
}
