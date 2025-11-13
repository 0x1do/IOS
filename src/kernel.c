#include "kernel.h"

__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = { MULTIBOOT_MAGIC,
										  0x0, // flags
										  MULTIBOOT_CHECKSUM };

struct DescriptorEntry encodeDescriptor(unsigned int SegLimit,
										int *BaseAddress,
										unsigned int type,
										bool S,
										unsigned int DPL,
										bool P,
										bool AVL,
										bool L,
										bool D_B,
										bool G)
{
	struct DescriptorEntry entry;

	entry.S = S;
	entry.P = P;
	entry.AVL = AVL;
	entry.L = L;
	entry.D_B = D_B;
	entry.G = G;
	entry.DPL = DPL;
	entry.Type = type & 0x0f;

	entry.SegLimit15_00 = SegLimit & 0xffff;
	entry.SegLimit19_16 = (SegLimit >> 16) & 0xf;

	entry.Base23_00 = ((unsigned int)BaseAddress & 0x00ffffff);
	entry.Base31_24 = ((unsigned int)BaseAddress >> 24) & 0xff;

	return entry;
}

void printDescriptorEntry(struct DescriptorEntry entry)
{
	printk("Segment Limit (15-00): 0x%x\n", entry.SegLimit15_00);
	printk("Base Address (23-00): 0x%x\n", entry.Base23_00);
	printk("Type: %u\n", entry.Type);
	printk("S (System): %u\n", entry.S);
	printk("DPL (Descriptor Privilege Level): %u\n", entry.DPL);
	printk("P (Present): %u\n", entry.P);
	printk("Segment Limit (19-16): 0x%x\n", entry.SegLimit19_16);
	printk("AVL (Available): %u\n", entry.AVL);
	printk("L (64-bit Code Segment): %u\n", entry.L);
	printk("D/B (Default Operation Size): %u\n", entry.D_B);
	printk("G (Granularity): %u\n", entry.G);
	printk("Base Address (31-24): 0x%x\n", entry.Base31_24);
}

void kernel_main(void)
{
	while (1) {
		__asm__("hlt");
	}
}
