// kernel.c — simplest kernel

__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,  // magic number
    0x0,         // flags
    -(0x1BADB002) // checksum
};

void kernel_main(void) {
    const char *msg = "Eli the bootloader works!!\n";
    char *vga = (char*)0xB8000; // VGA text buffer

    for (int i = 0; msg[i] != '\0'; i++) {
        vga[i * 2] = msg[i];      // character
        vga[i * 2 + 1] = 0x07;    // attribute: light grey on black
    }

    while (1) { __asm__("hlt"); } // halt CPU forever
}
