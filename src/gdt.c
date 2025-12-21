#include "gdt.h"
#include "printk.h"
#include <stddef.h>

extern void reloadSegments(void);

struct GdtDescriptorEntry
encodeGlobalDescriptor(unsigned int segment_limit,
					   void *base_address,
					   unsigned int type,
					   short system,
					   unsigned int descriptor_privilege_level,
					   short present,
					   short available,
					   short long_mode,
					   short d_b,
					   short granularity)
{
	struct GdtDescriptorEntry entry = {
		.system = system,
		.present = present,
		.available = available,
		.long_mode = long_mode,
		.d_b = d_b,
		.granularity = granularity,
		.descriptor_privilege_level = descriptor_privilege_level,
		.Type = type & 0x0f,

		.lower_segment_limit = segment_limit & 0xffff,
		.higher_segment_limit = (segment_limit >> 16) & 0xf,

		.lower_base_address = ((unsigned long)base_address & 0x00ffffff),
		.higher_base_address = ((unsigned long)base_address >> 24) & 0xff
	};
	return entry;
}

void printGdtDescriptorEntry(struct GdtDescriptorEntry entry)
{
	printk("============================================\n");
	printk("               GDT entry                     \n");
	printk("============================================\n");
	printk("Segment Limit (15-00): 0x%x\n", entry.lower_segment_limit);
	printk("Base Address (23-00): 0x%x\n", entry.lower_base_address);
	printk("Type: %u\n", entry.Type);
	printk("S (System): %u\n", entry.system);
	printk("DPL (Descriptor Privilege Level): %u\n",
		   entry.descriptor_privilege_level);
	printk("P (Present): %u\n", entry.present);
	printk("Segment Limit (19-16): 0x%x\n", entry.higher_segment_limit);
	printk("AVL (Available): %u\n", entry.available);
	printk("L (64-bit Code Segment): %u\n", entry.long_mode);
	printk("D/B (Default Operation Size): %u\n", entry.d_b);
	printk("G (Granularity): %u\n", entry.granularity);
	printk("Base Address (31-24): 0x%x\n", entry.higher_base_address);
}

struct Gdt gdt;
struct Gdtr gdtr;

void initGdt()
{
	gdtr.base_address = (unsigned long long)&gdt;
	gdtr.limit = (short)sizeof(gdt) - 1;

	/*
	 * todo: define values instead of hardcoding values
	 */

	gdt.empty = ENCODE_64_BIT_DESCRIPTOR(0, 0, 0, 0, 0, 0, 0);
	gdt.kernel_code = ENCODE_64_BIT_DESCRIPTOR(EXECUTE_READ, 1, 0, 1, 0, 0, 1);
	gdt.kernel_data = ENCODE_64_BIT_DESCRIPTOR(READ_WRITE, 1, 0, 1, 0, 1, 1);

	printGdtDescriptorEntry(gdt.kernel_code);
	printk("GDTR base = %p, limit = %d\nGDTR address = %p\n",
		   gdtr.base_address,
		   (int)gdtr.limit,
		   &gdtr);
	__asm__ volatile("lgdt %0" : : "m"(gdtr));
	__asm__ volatile("pushq %0\n"
					 "leaq reload_CS(%%rip), %%rax\n"
					 "pushq %%rax\n"
					 "retfq\n"

					 "reload_CS:\n"
					 "movw $0x10, %%ax\n"
					 "movw %%ax, %%ds\n"
					 "movw %%ax, %%es\n"
					 "movw %%ax, %%fs\n"
					 "movw %%ax, %%gs\n"
					 "movw %%ax, %%ss\n"
					 :
					 : "i"(offsetof(struct Gdt, kernel_code))
					 : "rax", "ax", "memory");
}