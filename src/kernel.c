#include "kernel.h"

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
	printk("GDTR base = %p, limit = %lx\n", gdtr.base_address, gdtr.limit);
}