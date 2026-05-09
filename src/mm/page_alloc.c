#include "page_alloc.h"
#include "mem.h"

static uint64_t first_page;
static uint64_t last_page = PHYSICAL_MEMORY_SIZE / PAGE_SIZE;
uint8_t bitmap[BITMAP_SIZE];
uint16_t page_counts[BITMAP_SIZE * 8];

void initAllocator()
{
	first_page = ((uint64_t)&endkernel + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	for (int i = 0; i < BITMAP_SIZE; i++) {
		bitmap[i] = FREE;
	}
	for (int i = 0; i < BITMAP_SIZE * 8; i++) {
		page_counts[i] = 0;
	}
	bitmap[0] |= (1 << 0);
}

/* Finds the first freed page(s) in the bitmap and returns its address
 * It will allocate the amount of pages closest to @size (rounding up)
 */
void *kalloc(size_t size)
{
	if (size == 0)
		return NULL;

	int pages_needed =
		(size < PAGE_SIZE) ? 1 : (size + PAGE_SIZE - 1) / PAGE_SIZE;
	int total_pages = BITMAP_SIZE * 8;
	int run_start = 0;
	int run_len = 0;

	for (int p = 0; p < total_pages; p++) {
		if ((bitmap[p / 8] & (1 << (p % 8))) == FREE) {
			if (run_len == 0)
				run_start = p;
			if (++run_len == pages_needed) {
				for (int i = run_start; i < run_start + pages_needed; i++)
					bitmap[i / 8] |= (1 << (i % 8));
				page_counts[run_start] = pages_needed;
				return (void *)((uint64_t)run_start * PAGE_SIZE + first_page);
			}
		} else {
			run_len = 0;
		}
	}

	/* No free pages found */
	return NULL;
}

void *krealloc(void *addr, uint64_t new_size)
{
	if (new_size == 0) {
		page_free(addr);
		return NULL;
	}

	uint32_t page_idx = ((uint64_t)addr - first_page) / PAGE_SIZE;
	uint16_t old_pages = page_counts[page_idx];
	uint16_t new_pages =
		(new_size < PAGE_SIZE) ? 1 : (new_size + PAGE_SIZE - 1) / PAGE_SIZE;

	/* Shrink in-place: free trailing pages */
	if (new_pages <= old_pages) {
		for (uint32_t i = page_idx + new_pages; i < page_idx + old_pages; i++)
			bitmap[i / 8] &= ~(1 << (i % 8));
		page_counts[page_idx] = new_pages;
		return addr;
	}

	/* Grow: allocate new, copy, free old */
	void *new_addr = kalloc(new_size);
	if (!new_addr)
		return NULL;
	memcpy(new_addr, addr, old_pages * PAGE_SIZE);
	page_free(addr);
	return new_addr;
}

void page_free(void *addr)
{
	if ((uint64_t)addr >= first_page &&
		(uint64_t)addr < (first_page + PHYSICAL_MEMORY_SIZE)) {
		uint32_t page_num = ((uint64_t)addr - first_page) / PAGE_SIZE;
		uint16_t count = page_counts[page_num];

		for (uint32_t i = page_num; i < page_num + count; i++) {
			bitmap[i / 8] &= ~(1 << (i % 8));
		}
		page_counts[page_num] = 0;
	}
}
