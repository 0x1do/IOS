#pragma once
#define MULTIBOOT_MAGIC 0x1BADB002
#define MULTIBOOT_CHECKSUM -(0x1BADB002)
#include "../framework/printk.h"
typedef enum { false = 0, true = 1 } bool;

void kernel_main(void);
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
void printDescriptorEntry(struct DescriptorEntry entry);

struct DescriptorEntry {
	unsigned int SegLimit15_00 : 16; // Segment Limit 15-00
	unsigned int Base23_00 : 24;	 // Base Address 23-00
	unsigned int Type : 4;			 // Type
	unsigned int S : 1;				 // S
	unsigned int DPL : 2;			 // DPL
	unsigned int P : 1;				 // P
	unsigned int SegLimit19_16 : 4;	 // Seg. Limit 19-16
	unsigned int AVL : 1;			 // AVL
	unsigned int L : 1;				 // L
	unsigned int D_B : 1;			 // D/B
	unsigned int G : 1;				 // G
	unsigned int Base31_24 : 8;		 // Base 31-24
};
