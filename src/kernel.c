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

int allocator_selftest(void)
{
	int fails = 0;

	if (allocator_init() != 0) {
		printk("[alloc-test] FAIL: allocator_init\n");
		return 1;
	}

	size_t sizes[] = { 16,	32,	  64,	128,  192,	256,
					   512, 1024, 2048, 4096, 4097, 6000 };

	for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
		size_t n = sizes[i];

		void *a = kmalloc(n);
		void *b = kmalloc(n);

		long long delta = 0;
		if (a && b)
			delta = (long long)((uintptr_t)b - (uintptr_t)a);

		printk("[alloc-test] size=%lu a=%p b=%p delta=%ld\n", n, a, b, delta);

		if (!a || !b) {
			printk("[alloc-test] FAIL: NULL alloc (size=%lu)\n", n);
			if (a)
				kfree(a);
			if (b)
				kfree(b);
			fails++;
			continue;
		}

		if (a == b) {
			printk("[alloc-test] FAIL: duplicate live pointers (size=%lu)\n",
				   n);
			fails++;
		}

		/* Alignment check: require at least pointer-size alignment; max_align_t
		 * if you want stricter. */
		if (((uintptr_t)a % (uintptr_t) _Alignof(max_align_t)) != 0) {
			printk("[alloc-test] FAIL: a not max_align_t aligned (size=%lu)\n",
				   n);
			fails++;
		}
		if (((uintptr_t)b % (uintptr_t) _Alignof(max_align_t)) != 0) {
			printk("[alloc-test] FAIL: b not max_align_t aligned (size=%lu)\n",
				   n);
			fails++;
		}

		/* Edge pattern check to detect overlap / bad sizing. */
		{
			uint8_t *pa = (uint8_t *)a;
			uint8_t *pb = (uint8_t *)b;

			pa[0] = 0xA1;
			pa[n - 1] = 0xA2;
			pb[0] = 0xB1;
			pb[n - 1] = 0xB2;

			if (pa[0] != 0xA1 || pa[n - 1] != 0xA2) {
				printk(
					"[alloc-test] FAIL: a corrupted after b write (size=%lu)\n",
					n);
				fails++;
			}
			if (pb[0] != 0xB1 || pb[n - 1] != 0xB2) {
				printk("[alloc-test] FAIL: b corrupted (size=%lu)\n", n);
				fails++;
			}
		}

		kfree(a);
		kfree(b);

		/* Reuse observation (informational only). */
		void *c = kmalloc(n);
		void *d = kmalloc(n);
		printk("[alloc-test] size=%lu reuse c=%p d=%p (info)\n", n, c, d);

		/* Invalid-free probe (informational; bad-free log is expected). */
		if (c) {
			printk("[alloc-test] size=%lu invalid-free probe: free(c+1)\n", n);
			kfree((void *)((uintptr_t)c + 1));
		}

		if (c)
			kfree(c);
		if (d)
			kfree(d);
	}

	printk("[alloc-test] end fails=%d\n", fails);
	return fails ? 1 : 0;
}

void kernelMain(void)
{
	initGdt();
	initIdt();
	allocator_init();

	serial_init(COM1_BASE, 1);
	clearKeyboardBuffer();

	/* Show GUI node selection splash before shell */
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
		msg_set_id(node_id);
	}

	printk("===================================\n");
	initFs();
}
