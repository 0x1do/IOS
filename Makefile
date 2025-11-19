CC = gcc
LD = ld
CFLAGS = -ffreestanding -nostdlib -m64 -g -mcmodel=large -I./src -I./src/framework -I./Limine -I./Flanterm/src -I./Flanterm/src/flanterm_backends -Wextra -Werror

ISO_DIR = iso
BUILD_DIR = build
KERNEL = kernel.elf
ISO = $(ISO_DIR)/ios.iso
LIMINE = Limine
LIMINE_BINARIES = limine-bios-cd.bin limine-uefi-cd.bin limine-bios.sys limine.conf
SRC := $(wildcard src/**/**/**/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/*.c) $(wildcard src/*.c) $(wildcard Flanterm/src/*.c) $(wildcard Flanterm/src/**/*.c)
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

all: $(ISO)

$(KERNEL): $(OBJS)
	mkdir -p $(ISO_DIR)/boot
	$(LD) -m elf_x86_64 -T src/boot/linker.ld -o $(KERNEL) $(OBJS)
	mkdir -p $(ISO_DIR)/boot
	mv $(KERNEL) $(ISO_DIR)/boot/.

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(ISO): $(KERNEL)
	mkdir -p $(BUILD_DIR)
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
	rm -r $(BUILD_DIR)/*
	rm -r $(ISO_DIR)/*
	
