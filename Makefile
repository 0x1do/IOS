CC = gcc
LD = ld
CFLAGS = -ffreestanding -nostdlib -m64 -g -mcmodel=large -I./src -I./src/framework -I./Limine -Wextra -Werror

ISO_DIR = iso
OUT_DIR = out
KERNEL = kernel.elf
ISO = $(OUT_DIR)/ios.iso
LIMINE = Limine
LIMINE_BINARIES = limine-bios-cd.bin limine-uefi-cd.bin limine-bios.sys limine.conf
SRC := $(wildcard src/**/**/**/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/*.c) $(wildcard src/*.c)
OBJS = $(SRC:.c=.o)

all: $(ISO)

$(KERNEL): $(OBJS)
	$(LD) -m elf_x86_64 -T boot/linker.ld -o $(KERNEL) $(OBJS)
	mv $(KERNEL) $(ISO_DIR)/boot/.

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ISO): $(KERNEL)
	mkdir -p $(OUT_DIR)
	@cp $(addprefix $(LIMINE)/,$(LIMINE_BINARIES)) $(ISO_DIR)/
	cp $(LIMINE)/limine.conf .
	xorriso -as mkisofs \
		-b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		-eltorito-alt-boot \
		-e limine-uefi-cd.bin \
		-no-emul-boot \
		-isohybrid-mbr $(LIMINE)/limine-bios.sys \
		-isohybrid-gpt-basdat \
		-o $(ISO) $(ISO_DIR)

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO)

clean:
	rm -f $(OBJS) $(KERNEL) $(ISO_DIR)/boot/$(KERNEL)
	rm $(ISO)
	rm -rf $(OUT_DIR)
	rm $(ISO_DIR)/limine*
