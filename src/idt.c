#include "idt.h"

struct IdtTable *idt_table;

void encodeInterruptDescriptor(size_t *offset,
							   short segment_selector,
							   enum GateType gate_type,
							   char dpl,
							   char p)
{
	struct IdtDescriptorEntry entry = {
		.lower_offset = (unsigned int)offset & 0xffff,
		.segment_selector = segment_selector,
		.interrupt_stack_table = 0, // for now IST is not implemented so the
									// entry for the stack will rely on DPL
		.reserved = 0,
		.gate_type = gate_type,
		.reserved_1 = 0,
		.descriptor_privilege_level = dpl,
		.present = p,
		.higher_offset = ((unsigned int)offset >> 16) & 0xffff,
		.reserved_2 = 0
	};
	return entry;
}

void initIdt()
{
	struct Idtr idtr = {
		.limit = IDT_MAX_ENTRIES * sizeof(struct IdtDescriptorEntry) - 1,
		.base_address = &idt_table
	};

	__asm__ volatile("lidt %0" : : "m"(idtr));
}