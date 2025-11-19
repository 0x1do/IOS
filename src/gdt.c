#include "gdt.h"

struct DescriptorEntry encodeDescriptor(unsigned int segment_limit,
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
	struct DescriptorEntry entry;

	entry.system = system;
	entry.present = present;
	entry.available = available;
	entry.long_mode = long_mode;
	entry.d_b = d_b;
	entry.granularity = granularity;
	entry.descriptor_privilege_level = descriptor_privilege_level;
	entry.Type = type & 0x0f;

	entry.lower_segment_limit = segment_limit & 0xffff;
	entry.higher_segment_limit = (segment_limit >> 16) & 0xf;

	entry.lower_base_address = ((unsigned long)base_address & 0x00ffffff);
	entry.higher_base_address = ((unsigned long)base_address >> 24) & 0xff;

	return entry;
}

void printDescriptorEntry(struct DescriptorEntry entry)
{
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

struct DescriptorEntry
encode64BitDescriptor(unsigned int type,
					  short system,
					  unsigned int descriptor_privilege_level,
					  short present,
					  short available,
					  short d_b,
					  short granularity)
{
	return encodeDescriptor(0xfffff,
							0,
							type,
							system,
							descriptor_privilege_level,
							present,
							available,
							1,
							d_b,
							granularity);
}

struct Gdtr gdtr;
struct GdtTable gdt_table;

void initGdt()
{
	gdtr.base_address = (void *)&gdt_table;
	gdtr.limit = sizeof(gdt_table);

	gdt_table.table[EMPTY_ENTRY] = encode64BitDescriptor(0, 0, 0, 0, 0, 0, 0);
	gdt_table.table[CODE_SEGMENT] =
		encode64BitDescriptor(EXECUTE_READ, 1, 3, 1, 0, 1, 1);
	gdt_table.table[DATA_SEGMENT] =
		encode64BitDescriptor(READ_WRITE, 1, 3, 1, 0, 1, 1);
}