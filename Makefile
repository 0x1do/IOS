CC = gcc
LD = ld
CFLAGS = -ffreestanding -nostdlib -m32 -Werror

ISO_DIR = iso
OUT_DIR = out
KERNEL = kernel.elf
ISO = $(OUT_DIR)/ios.iso

SRC := $(wildcard src/**/*.c) $(wildcard src/*.c)
OBJS = $(SRC:.c=.o)

all: $(ISO)

$(KERNEL): $(OBJS)
	$(LD) -m elf_i386 -T boot/linker.ld -o $(KERNEL) $(OBJS)
	mv $(KERNEL) $(ISO_DIR)/boot/.

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ISO): $(KERNEL)
	mkdir -p $(OUT_DIR)
	grub-mkrescue -o $(ISO) $(ISO_DIR)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

clean:
	rm -f $(OBJS) $(KERNEL) $(ISO_DIR)/boot/$(KERNEL)
	rm -rf $(OUT_DIR)
