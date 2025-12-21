#pragma once
#include "kernel.h"
#define IDT_MAX_ENTRIES 256

struct IdtDescriptorEntry {
	unsigned int lower_offset : 16;
	unsigned int segment_selector : 16;
	unsigned int interrupt_stack_table : 3;
	unsigned int reserved : 5;
	unsigned int gate_type : 4;
	unsigned int reserved_1 : 1;
	unsigned int descriptor_privilege_level : 2;
	unsigned int present : 1;
	unsigned int middle_offset : 16;      
	unsigned long long higher_offset : 32;
	unsigned int reserved_2 : 32;
} __attribute__((packed));

static_assert(sizeof(struct IdtDescriptorEntry) == 16,
			  "IdtDescriptorEntry size is not 16 bytes");

void initIdt();
void printIdtDescriptorEntry(struct IdtDescriptorEntry entry);

enum GateType { INTERRUPT_GATE = 0XE, TRAP_GATE = 0XF };

struct Idtr {
	short limit;
	unsigned long long base_address;

} __attribute__((packed));

struct IdtDescriptorEntry encodeInterruptDescriptor(void *offset,
													short segment_selector,
													enum GateType gate_type,
													char dpl,
													char p);

extern struct IdtDescriptorEntry idtTable[IDT_MAX_ENTRIES];
extern struct Idtr idtr;
