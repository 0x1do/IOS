# CC = gcc
# LD = ld
# CFLAGS = -ffreestanding -nostdlib -m32

# all: kernel.elf

# kernel.elf: src/kernel.c
# 	$(CC) $(CFLAGS) -c src/kernel.c -o kernel.o
# 	$(LD) -m elf_i386 -T boot/linker.ld -o kernel.elf kernel.o
# 	mv kernel.elf iso/boot/.
# 	mkdir out
# 	grub-mkrescue -o out/ios.iso iso
# 	qemu-system-i386 -cdrom out/ios.iso

# clean:
# 	rm -f *.o kernel.elf iso/boot/kernel.elf
# 	rm -rf out
CC = gcc
LD = ld
CFLAGS = -ffreestanding -nostdlib -m32

ISO_DIR = iso
OUT_DIR = out
KERNEL = kernel.elf
ISO = $(OUT_DIR)/ios.iso

all: $(ISO)

$(KERNEL): src/kernel.c
	$(CC) $(CFLAGS) -c src/kernel.c -o kernel.o
	$(LD) -m elf_i386 -T boot/linker.ld -o $(KERNEL) kernel.o
	mv $(KERNEL) $(ISO_DIR)/boot/.

$(ISO): $(KERNEL)
	mkdir -p $(OUT_DIR)
	grub-mkrescue -o $(ISO) $(ISO_DIR)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

clean:
	rm -f *.o $(KERNEL) $(ISO_DIR)/boot/$(KERNEL)
	rm -rf $(OUT_DIR)
