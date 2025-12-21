#pragma once

struct GdtDescriptorEntry
encodeGlobalDescriptor(unsigned int segment_limit,
					   void *base_address,
					   unsigned int Type,
					   short system,
					   unsigned int descriptor_privilege_level,
					   short present,
					   short available,
					   short long_mode,
					   short d_b,
					   short granularity);

#define ENCODE_64_BIT_DESCRIPTOR(type,                                         \
								 system,                                       \
								 descriptor_privilege_level,                   \
								 present,                                      \
								 available,                                    \
								 d_b,                                          \
								 granularity)                                  \
	encodeGlobalDescriptor(0xfffff,                                            \
						   0,                                                  \
						   type,                                               \
						   system,                                             \
						   descriptor_privilege_level,                         \
						   present,                                            \
						   available,                                          \
						   1,                                                  \
						   d_b,                                                \
						   granularity)

void printGdtDescriptorEntry(struct GdtDescriptorEntry entry);
void initGdt();

struct GdtDescriptorEntry {
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

static_assert(sizeof(struct GdtDescriptorEntry) == 8,
			  "DescriptorEntry size is not 8 bytes");

struct Gdt {
	struct GdtDescriptorEntry empty;
	struct GdtDescriptorEntry kernel_code;
	struct GdtDescriptorEntry kernel_data;
} __attribute__((packed));

struct Gdtr {
	short limit;
	unsigned long long base_address;
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

extern struct Gdtr gdtr;
extern struct GdtDescriptorEntry table[8];
