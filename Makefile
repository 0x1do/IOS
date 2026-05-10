CC = gcc-14
LD = ld
CFLAGS = -std=gnu23 -ffreestanding -nostdlib -m64 -g -mcmodel=large -I./src -I./src/framework -I./Limine -I./Flanterm/src -I./Flanterm/src/flanterm_backends -I./src/mm -I./src/fs -Wextra -Werror

# Host-side tools (the routing hub) build with normal hosted libc.
HOST_CC = gcc
HOST_CFLAGS = -O2 -g -Wall -Wextra

LINK_SOCK := /tmp/ioslink.sock

ISO_DIR = iso
BUILD_DIR = build
SRC_DIR = src
KERNEL = kernel.elf
ISO = $(ISO_DIR)/ios.iso
LIMINE = Limine
LIMINE_BINARIES = limine-bios-cd.bin limine-uefi-cd.bin limine-bios.sys limine.conf
SRC := $(wildcard src/**/**/**/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/*.c) $(wildcard src/*.c) $(wildcard Flanterm/src/*.c) $(wildcard Flanterm/src/**/*.c)
ASM_SRC := $(wildcard src/**/**/**/*.s) $(wildcard src/**/**/*.s) $(wildcard src/**/*.s) $(wildcard src/*.s)
ASM_OBJECT = $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(ASM_SRC))

C_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))
OBJS := $(ASM_OBJECT) $(C_OBJS)

HUB_SRC = scripts/ioslink-hub.c
HUB_BIN = scripts/ioslink-hub

all: $(ISO)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(dir $@)
	@nasm -felf64 -F dwarf -g -o $@ $<

$(KERNEL): $(OBJS)
	mkdir -p $(ISO_DIR)/boot
	$(LD) -m elf_x86_64 -T src/boot/linker.ld -o $(KERNEL) $(OBJS)
	mkdir -p $(ISO_DIR)/boot
	mv $(KERNEL) $(ISO_DIR)/boot/.

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
	qemu-system-x86_64 -D log.txt -d int -cdrom $(ISO)

debug: $(ISO)
	qemu-system-x86_64 -D log.txt -d int -cdrom $(ISO) -s -S

$(HUB_BIN): $(HUB_SRC)
	$(HOST_CC) $(HOST_CFLAGS) -o $@ $<

hub: $(HUB_BIN)
	$(HUB_BIN) --sock $(LINK_SOCK) -v

conn: $(ISO) $(HUB_BIN)
	HUB=$(HUB_BIN) ./scripts/qemu-conn.sh

conn-debug: $(ISO) $(HUB_BIN)
	HUB=$(HUB_BIN) ./scripts/qemu-conn.sh -s -S

clean:
	rm -r $(BUILD_DIR)/*
	rm -r $(ISO_DIR)/*
	rm -f $(HUB_BIN)

.PHONY: all run debug hub conn conn-debug clean
