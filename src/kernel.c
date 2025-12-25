#include "kernel.h"
#include "flanterm_utils.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "page_alloc.h"
#include "printk.h"

__attribute__((used, section(".limine_requests"))) static volatile uint64_t
	limine_base_revision[] = LIMINE_BASE_REVISION(4);

__attribute__((used,
			   section(".limine_requests_start"))) static volatile uint64_t
	limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_start"))) static volatile struct
	limine_framebuffer_request framebuffer_request = {
		.id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0
	};

__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t
	limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

struct flanterm_context *ft_ctx;

void _start(void)
{
	ft_ctx = initTerminal(framebuffer_request);
	kernelMain();
	while (1) {
		__asm__("hlt");
	}
}

void kernelMain(void)
{
	initGdt();
	initIdt();
	initAllocator();

	__asm__ volatile("int $3");

	char buf[50];

	/* Test Case 1: Basic String */
	scanf("In: %s", buf);
	/* Write: hello */
	printk("Out: >%s< should be >hello<\n\n", buf);

	/* Test Case 2: String with space (Only first word should be captured) */
	scanf("In: %s", buf);
	/* Write: hello world */
	printk("Out: >%s< should be >hello<\n\n", buf);

	/* Test Case 3: Empty string (No input, should handle empty) */
	scanf("In: %s", buf);
	/* Write: (just press Enter with no characters) */
	printk("Out: >%s< should be >(empty string)<\n\n", buf);

	/* Test Case 4: String with leading spaces (should ignore leading spaces) */
	scanf("In: %s", buf);
	/* Write:     hello */
	printk("Out: >%s< should be >hello<\n\n", buf);

	/* Test Case 5: Numeric input (should work like a regular string) */
	scanf("In: %s", buf);
	/* Write: 1234 */
	printk("Out: >%s< should be >1234<\n\n", buf);

	/* Test Case 6: Mixed alphanumeric input */
	scanf("In: %s", buf);
	/* Write: hello123 */
	printk("Out: >%s< should be >hello123<\n\n", buf);

	/* Test Case 7: Input with multiple spaces (Only first word should be
	captured) */
	scanf("In: %s", buf);
	/* Write: hello   world    universe */
	printk("Out: >%s< should be >hello<\n\n", buf);

	/* Test Case 8: Input with newline character after it (newline is not
	captured by `%s`) */
	scanf("In: %s", buf);
	/* Write: hello\n (press Enter after typing) */
	printk("Out: >%s< should be >hello<\n\n", buf);

	/* Test Case 9: Input with trailing spaces (should stop at the first space)
	 */
	scanf("In: %s", buf);
	/* Write: hello */
	printk("Out: >%s< should be >hello<\n\n", buf);

	/* Test Case 10: Very long input (check if the buffer overflows or truncates
	 properly) */
	scanf("In: %s", buf);
	/* Write: a-very-long-string-that-is-definitely-too-long-for-the-buffer */
	printk(
		"Out: >%s< should be "
		">a-very-long-string-that-is-definitely-too-long-for-the-buffer<\n\n",
		buf);
}