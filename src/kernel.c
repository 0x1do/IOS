#include "kernel.h"
#include "allocator.h"
#include "connection.h"
#include "fs.h"
#include "flanterm_utils.h"
#include "fs/shell.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "page_alloc.h"
#include "printk.h"
#include "splash.h"
#include "msg.h"

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

__attribute__((noreturn)) void _exit(int status)
{
	asm volatile("cli");
	for (;;) {
		asm volatile("hlt");
	}
}

void _start(void)
{
	ft_ctx = initTerminal(framebuffer_request);
	kernelMain();
	_exit(0);
}

void kernelMain(void)
{
	initGdt();
	initIdt();

	serial_init(COM1_BASE, 1);
	clearKeyboardBuffer();


	printk("===================================\n");
	initFs();
}
