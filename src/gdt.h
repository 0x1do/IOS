#pragma once
#include "kernel.h"

struct GdtDescriptorEntry
encodeGlobalDescriptor(uint32_t segment_limit,
					   void *base_address,
					   uint32_t Type,
					   uint16_t system,
					   uint32_t descriptor_privilege_level,
					   uint16_t present,
					   uint16_t available,
					   uint16_t long_mode,
					   uint16_t d_b,
					   uint16_t granularity);

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
	uint32_t lower_segment_limit : 16;
	uint32_t lower_base_address : 24;
	uint32_t Type : 4;
	uint32_t system : 1;
	uint32_t descriptor_privilege_level : 2;
	uint32_t present : 1;
	uint32_t higher_segment_limit : 4;
	uint32_t available : 1;
	uint32_t long_mode : 1;
	uint32_t d_b : 1;
	uint32_t granularity : 1;
	uint32_t higher_base_address : 8;
} __attribute__((packed));

static_assert(sizeof(struct GdtDescriptorEntry) == 8,
			  "DescriptorEntry size is not 8 bytes");

struct Gdt {
	struct GdtDescriptorEntry empty;
	struct GdtDescriptorEntry kernel_code;
	struct GdtDescriptorEntry kernel_data;
} __attribute__((packed));

struct Gdtr {
	uint16_t limit;
	uint64_t base_address;
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
