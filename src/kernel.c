#include "kernel.h"

__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    MULTIBOOT_MAGIC,
    0x0, // flags
    MULTIBOOT_CHECKSUM};

void printk(char *str)
{
    static int offset = 0;
    while (*str != '\0')
    {
        vga->chr = *str;
        vga->clr = LIGHT_GREY_ON_BLACK;
        str++;
        vga++;
    }
}

void kernel_main(void)
{
    printk("simplest ");
    printk("printk");
    while (1)
    {
        __asm__("hlt");
    }
}
