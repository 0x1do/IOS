#pragma once
#define LIGHT_GREY_ON_BLACK 0x07
#define MULTIBOOT_MAGIC 0x1BADB002
#define MULTIBOOT_CHECKSUM -(0x1BADB002)
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define MAX_SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define VGA_MEMORY 0xB8000

struct vga_char {
    char character;
    char attribute;
};

void printk(char *str);
void kernel_main(void);
