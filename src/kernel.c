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
	allocator_init();

	serial_init(COM1_BASE, 1);
	clearKeyboardBuffer();

	if (framebuffer_request.response && framebuffer_request.response->framebuffer_count > 0) {
		struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

		SplashFB sfb = {
			.fb = (uint32_t *)fb->address,
			.width = fb->width,
			.height = fb->height,
			.pitch4 = fb->pitch / 4,
			.r_shift = fb->red_mask_shift,
			.g_shift = fb->green_mask_shift,
			.b_shift = fb->blue_mask_shift
		};

		int node_id = splash_screen(&sfb);
		/* Reset flanterm: clear screen + home cursor, since prior boot
		 * prints had advanced its cursor before the splash drew over them. */
		printk("\x1b[2J\x1b[H");
		msg_set_id(node_id);
		printk("Node %d selected!\n", node_id);
	}

	initFs();
}
