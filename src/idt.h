#pragma once
#include "kernel.h"
#define IDT_MAX_ENTRIES 256

struct IdtDescriptorEntry {
	uint32_t lower_offset : 16;
	uint32_t segment_selector : 16;
	uint32_t interrupt_stack_table : 3;
	uint32_t reserved : 5;
	uint32_t gate_type : 4;
	uint32_t reserved_1 : 1;
	uint32_t descriptor_privilege_level : 2;
	uint32_t present : 1;
	uint32_t middle_offset : 16;
	uint64_t higher_offset : 32;
	uint32_t reserved_2 : 32;
} __attribute__((packed));

static_assert(sizeof(struct IdtDescriptorEntry) == 16,
			  "IdtDescriptorEntry size is not 16 bytes");

void initIdt();
void printIdtDescriptorEntry(struct IdtDescriptorEntry entry);

enum GateType { INTERRUPT_GATE = 0XE, TRAP_GATE = 0XF };

struct Idtr {
	uint16_t limit;
	uint64_t base_address;

} __attribute__((packed));

struct IdtDescriptorEntry encodeInterruptDescriptor(void *offset,
													uint16_t segment_selector,
													enum GateType gate_type,
													char dpl,
													char p);

extern struct IdtDescriptorEntry idtTable[IDT_MAX_ENTRIES];
extern struct Idtr idtr;
