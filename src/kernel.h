#pragma once
#define LIGHT_GREY_ON_BLACK 0x07
#define MULTIBOOT_MAGIC 0x1BADB002
#define MULTIBOOT_CHECKSUM -(0x1BADB002)

struct vga_char {
    char chr;
    char clr;
};
struct vga_char *vga = (struct vga_char *)0xB8000;
void printk(char *str);
void kernel_main(void);
