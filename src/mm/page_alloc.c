#include "page_alloc.h"
#include "mem.h"

static uint64_t first_page;
static uint64_t last_page = PHYSICAL_MEMORY_SIZE / PAGE_SIZE;
uint8_t bitmap[BITMAP_SIZE];

void initAllocator()
{
	first_page = (uint64_t)&endkernel;
	for (int i = 0; i < BITMAP_SIZE; i++) {
		bitmap[i] = FREE;
	}
	bitmap[0] |= (1 << 0);
}

/* Finds the first freed page(s) in the bitmap and returns its address
 * It will allocate the amount of pages closest to @size (rounding up)
 */
void *kalloc(int size)
{
	if (size == 0) {
		return NULL;
	}
	int pages_amount =
		(size < PAGE_SIZE) ? 1 : (size + PAGE_SIZE - 1) / PAGE_SIZE;
	int found_pages = 0;

	for (int i = 0; i < BITMAP_SIZE; i++) {
		for (int j = 0; j < 8; j++) {
			if ((bitmap[i] & (1 << j)) == FREE) {
				uint64_t page_num = (i * 8) + j;
				void *addr = (void *)(page_num * PAGE_SIZE + first_page);

				/* How many free pages in a row we can find */
				int k = 0;
				while (k < pages_amount) {
					int next_bit_index = (i * 8 + j + k) / 8;
					if (next_bit_index >= BITMAP_SIZE)
						break;
					int next_bit_offset = (i * 8 + j + k) % 8;

					if ((bitmap[next_bit_index] & (1 << next_bit_offset)) ==
						FREE) {
						k++;
					} else {
						break;
					}
				}

				if (k == pages_amount) {
					/* Mark these pages as allocated */
					for (int l = 0; l < pages_amount; l++) {
						int bit_index = (i * 8 + j + l) / 8;
						int bit_offset = (i * 8 + j + l) % 8;
						bitmap[bit_index] |= (1 << bit_offset);
					}
					uint16_t *size_field = (uint16_t *)addr;
					*size_field = pages_amount;
					return (void *)((uint64_t)addr + sizeof(uint16_t));
				}
			}
		}
	}

	/* No free pages found */
	return NULL;
}

void *krealloc(void *addr, uint64_t new_size)
{
	if (new_size == 0) {
		kfree(addr);
		return NULL;
	}
	uint64_t old_addr = (uint64_t)addr - sizeof(uint16_t);
	uint16_t old_size = *(uint16_t *)old_addr;
	uint64_t page_idx = (old_addr - first_page) / PAGE_SIZE;

	uint64_t new_pages =
		(new_size < PAGE_SIZE) ? 1 : (new_size + PAGE_SIZE - 1) / PAGE_SIZE;
	if (new_size <= (uint64_t)(PAGE_SIZE * (old_size + 1))) {
		for (uint64_t i = page_idx; i >= new_pages; i--) {
			kfree((void *)(i * PAGE_SIZE + first_page));
		}
		return addr;
	}

	void *new_addr = kalloc(new_size);
	if (new_addr == NULL) {
		return NULL;
	}

	memcpy(new_addr, addr, MIN(new_pages, old_size) * PAGE_SIZE);
	kfree(addr);
	return new_addr;
}

void kfree(void *addr)
{
	if ((uint64_t)addr >= first_page &&
		(uint64_t)addr < (first_page + PHYSICAL_MEMORY_SIZE)) {
		uint64_t chunk_start = (uint64_t)addr - sizeof(uint16_t);
		uint16_t size_field = *(uint16_t *)chunk_start;
		uint32_t page_num = (chunk_start - first_page) / PAGE_SIZE;

		for (uint32_t i = page_num;
			 i < page_num + ((size_field + PAGE_SIZE - 1) / PAGE_SIZE);
			 i++) {
			bitmap[i / 8] &= ~(1 << (i % 8));
		}
	}
}
