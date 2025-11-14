#pragma once
#include "kernel.h"

struct DescriptorEntry encodeDescriptor(unsigned int segment_limit,
										int *base_address,
										unsigned int Type,
										bool system,
										unsigned int descriptor_privilege_level,
										bool present,
										bool available,
										bool long_mode,
										bool d_b,
										bool granularity);

struct DescriptorEntry
encode64BitDescriptor(unsigned int Type,
					  bool system,
					  unsigned int descriptor_privilege_level,
					  bool present,
					  bool available,
					  bool d_b,
					  bool granularity);
void printDescriptorEntry(struct DescriptorEntry entry);
void initGDT();

struct DescriptorEntry {
	unsigned int lower_segment_limit : 16;
	unsigned int lower_base_address : 24;
	unsigned int Type : 4;
	unsigned int system : 1;
	unsigned int descriptor_privilege_level : 2;
	unsigned int present : 1;
	unsigned int higher_segment_limit : 4;
	unsigned int available : 1;
	unsigned int long_mode : 1;
	unsigned int d_b : 1;
	unsigned int granularity : 1;
	unsigned int higher_base_address : 8;
} __attribute__((packed));

struct GDTTable {
	struct DescriptorEntry table[8];
};

struct GDTR {
	int limit;
	int *base_address;
} __attribute__((packed));

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

enum TableEntryMeaning {
	EMPTY_ENTRY,
	CODE_SEGMENT,
	DATA_SEGMENT
};

extern struct GDTR gdtr;
extern struct GDTTable gdt_table;
