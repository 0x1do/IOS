#include "isr.h"
#include "kernel.h"
#include "mem.h"
#include "printk.h"
static void (*const isr_handlers[ISR_COUNT])(void) = {
	isr0,  isr1,  isr2,	 isr3,	isr4,  isr5,  isr6,	 isr7,	isr8,  isr9,  isr10,
	isr11, isr12, isr13, isr14, isr15, isr16, isr17, isr18, isr19, isr20, isr21,
	isr22, isr23, isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
};

static const char *exception_names[] = { "Division Error",
										 "Debug",
										 "Non-maskable Interrupt",
										 "Breakpoint",
										 "Overflow",
										 "Bound Range Exceeded",
										 "Invalid Opcode",
										 "Device Not Available",
										 "Double Fault",
										 "Coprocessor Segment Overrun",
										 "Invalid TSS",
										 "Segment Not Present",
										 "Stack-Segment Fault",
										 "General Protection Fault",
										 "Page Fault",
										 "Reserved",
										 "x87 Floating-Point Exception",
										 "Alignment Check",
										 "Machine Check",
										 "SIMD Floating-Point Exception",
										 "Virtualization Exception",
										 "Control Protection Exception",
										 "Reserved",
										 "Reserved",
										 "Reserved",
										 "Reserved",
										 "Reserved",
										 "Reserved",
										 "Hypervisor Injection Exception",
										 "VMM Communication Exception",
										 "Security Exception",
										 "Reserved" };

void isrHandler(int num)
{
	printk("\n========== EXCEPTION ==========\n");
	if (num < 32) {
		printk("Exception: %s (INT %d)\n", exception_names[num], num);
	} else {
		printk("Interrupt: %d\n", num);
	}
	printk("===============================\n");

	if (num > 32) {
		printk("System halted.\n");
		_exit(0);
	}
}

void initIsr(struct IdtDescriptorEntry (*)[256])
{
	for (int i = 0; i < ISR_COUNT; i++) {
		idtTable[i] =
			encodeInterruptDescriptor((void *)isr_handlers[i], 8, 14, 0, 1);
	}
}
