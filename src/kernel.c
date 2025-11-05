#include "kernel.h"

__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    MULTIBOOT_MAGIC,
    0x0, // flags
    MULTIBOOT_CHECKSUM};

static struct vga_char *const vga_buf = (struct vga_char *)VGA_MEMORY;

void printk(char *str)
{
    static int offset = 0;
    while (*str != '\0')
    {
        if (offset >= MAX_SCREEN_SIZE) {
            offset = 0;
        }
        if (*str == '\n') {
            offset += SCREEN_WIDTH - (offset % SCREEN_WIDTH) - 1;
        }
        vga_buf[offset].character = *str;
        vga_buf[offset].attribute = LIGHT_GREY_ON_BLACK;
        str++;
        offset++;
    }
}

void kernel_main(void)
{
    while (1)
    {
        __asm__("hlt");
    }
}
