#pragma once
#include "kernel.h"

struct DescriptorEntry encodeDescriptor(unsigned int SegLimit,
										int *BaseAddress,
										unsigned int Type,
										bool S,
										unsigned int DPL,
										bool P,
										bool AVL,
										bool L,
										bool D_B,
										bool G);

struct DescriptorEntry encode64BitDescriptor(unsigned int Type,
											 bool S,
											 unsigned int DPL,
											 bool P,
											 bool AVL,
											 bool D_B,
											 bool G);
void printDescriptorEntry(struct DescriptorEntry entry);
	void initGDT();

struct DescriptorEntry {
	unsigned int SegLimit15_00 : 16;
	unsigned int Base23_00 : 24;
	unsigned int Type : 4;
	unsigned int S : 1;
	unsigned int DPL : 2;
	unsigned int P : 1;
	unsigned int SegLimit19_16 : 4;
	unsigned int AVL : 1;
	unsigned int L : 1;
	unsigned int D_B : 1;
	unsigned int G : 1;
	unsigned int Base31_24 : 8;
} __attribute__((packed));

struct GDTTable {
	struct DescriptorEntry table[8];
};

struct GDTR {
	int limit;
	int *baseAddress;
}__attribute__((packed));

enum TypeField {
	READ_ONLY,
	READ_ONLY_ACCESSED,
    READ_WRITE,
    READ_WRITE_ACCESSED,
    READ_ONLY_EXPAND_DOWN,
    READ_ONLY_EXPAND_DOWN_ACCESSED,
    READ_WRITE_EXPAND_DOWN,
    READ_WRITE_EXPAND_DOWN_ACCESSED,
    EXECUTE_ONLY,
    EXECUTE_ONLY_ACCESSED,
    EXECUTE_READ,
    EXECUTE_READ_ACCESSED,
    EXECUTE_ONLY_CONFORMING,
    EXECUTE_ONLY_CONFORMING_ACCESSED,
    EXECUTE_READ_CONFORMING,
    EXECUTE_READ_CONFORMING_ACCESSED
};

extern struct GDTR gdtr;
extern struct GDTTable gdt_table;
