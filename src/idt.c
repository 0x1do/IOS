#include "idt.h"
#include "isr.h"
#include "mem.h"
#include "printk.h"

struct IdtDescriptorEntry idtTable[IDT_MAX_ENTRIES];

struct IdtDescriptorEntry encodeInterruptDescriptor(void *offset,
													short segment_selector,
													enum GateType gate_type,
													char dpl,
													char p)
{
	unsigned long addr = (unsigned long)offset;
	struct IdtDescriptorEntry entry = {
		.lower_offset = addr & 0xffff,
		.segment_selector = segment_selector,
		.interrupt_stack_table = 0, // for now IST is not implemented so the
									// entry for the stack will rely on DPL and
									// because I only have 1 ring
		.reserved = 0,
		.gate_type = gate_type,
		.reserved_1 = 0,
		.descriptor_privilege_level = dpl,
		.present = p,
		.middle_offset = (addr >> 16) & 0xffff,
		.higher_offset = (addr >> 32) & 0xffffffff,
		.reserved_2 = 0
	};
	return entry;
}

void printIdtDescriptorEntry(struct IdtDescriptorEntry entry)
{
	unsigned long full_offset = (unsigned long)entry.lower_offset |
		((unsigned long)entry.middle_offset << 16) |
		((unsigned long)entry.higher_offset << 32);
	printk("============================================\n");
	printk("               IDT entry                     \n");
	printk("============================================\n");
	printk("  Offset (full): 0x%lx\n", full_offset);
	printk("  Offset (lower 16 bits): 0x%x\n", entry.lower_offset);
	printk("  Offset (middle 16 bits): 0x%x\n", entry.middle_offset);
	printk("  Offset (higher 32 bits): 0x%lx\n", entry.higher_offset);
	printk("  Segment Selector: 0x%x\n", entry.segment_selector);
	printk("  Interrupt Stack Table (IST): %u\n", entry.interrupt_stack_table);
	printk("  Reserved: 0x%x\n", entry.reserved);
	printk("  Gate Type: 0x%x\n", entry.gate_type);
	printk("  Reserved_1: %u\n", entry.reserved_1);
	printk("  Descriptor Privilege Level (DPL): %u\n",
		   entry.descriptor_privilege_level);
	printk("  Present: %u\n", entry.present);
	printk("  Reserved_2: 0x%lx\n", entry.reserved_2);
}

void initIdt()
{
	memset(&idtTable, 0, sizeof(idtTable));
	initIsr(&idtTable);
	struct Idtr idtr = {
		.limit = IDT_MAX_ENTRIES * sizeof(struct IdtDescriptorEntry) - 1,
		.base_address = (unsigned long long)idtTable
	};

	__asm__ volatile("lidt %0" : : "m"(idtr));
	__asm__ volatile("sti");
	printk("Interrupts enabled (STI executed)\n");
}

