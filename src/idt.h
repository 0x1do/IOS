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
	unsigned long long higher_offset : 48;
	unsigned int reserved_2 : 32;
} __attribute__((packed));

void initIdt();
void printIdtDescriptorEntry(struct IdtDescriptorEntry entry);

enum GateType {
	INTERRUPT_GATE = 0XE,
	TRAP_GATE = 0XF
};

struct IdtTable {
	struct IdtDescriptorEntry table[IDT_MAX_ENTRIES];
} __attribute__((packed));

struct Idtr {
	unsigned int limit : 16; // calculated sizeof(entry)*entries-1, max value is
							 // 16*256-1=4095
	void *base_address;

} __attribute__((packed));

extern struct IdtTable *idt_table;
extern struct Idtr idtr;
