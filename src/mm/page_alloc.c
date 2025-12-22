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
					int next_bit_offset = (i * 8 + j + k) % 8;

					if ((bitmap[next_bit_index] & (1 << next_bit_offset)) ==
						FREE) {
						k++;
					} else {
						break;
					}
				}

				if (k == pages_amount) {
					// Mark these pages as allocated
					for (int l = 0; l < pages_amount; l++) {
						int bit_index = (i * 8 + j + l) / 8;
						int bit_offset = (i * 8 + j + l) % 8;
						bitmap[bit_index] |= (1 << bit_offset);
					}

					return addr;
				}
			}
		}
	}

	// No free pages found
	return NULL;
}

void *krealloc(void *addr, uint64_t new_size)
{
	uint64_t old_addr = (uint64_t)addr;
	uint64_t page_num = (old_addr - first_page) / PAGE_SIZE;

	int new_pages =
		(new_size < PAGE_SIZE) ? 1 : (new_size + PAGE_SIZE - 1) / PAGE_SIZE;
	if (new_size <= PAGE_SIZE * (page_num + 1)) {
		int old_pages = (page_num + 1);
		for (int i = old_pages - 1; i >= new_pages; i--) {
			kfree((void *)(i * PAGE_SIZE + first_page));
		}
		return addr;
	}

	void *new_addr = kalloc(new_size);
	if (new_addr == NULL) {
		return NULL;
	}

	memcpy(new_addr, addr, page_num * PAGE_SIZE);
	kfree(addr);
	return new_addr;
}

void kfree(void *addr)
{
	uint8_t page_num = ((uint64_t)addr - first_page) / PAGE_SIZE;
	bitmap[page_num / 8] &= ~(1 << (page_num % 8));
}
