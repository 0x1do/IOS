CC = gcc-14
LD = ld
CFLAGS = -std=gnu23 -ffreestanding -nostdlib -m64 -g -mcmodel=large -I./src -I./src/framework -I./Limine -I./Flanterm/src -I./Flanterm/src/flanterm_backends -I./src/mm -Wextra -Werror

ISO_DIR = iso
BUILD_DIR = build
SRC_DIR = src
KERNEL = kernel.elf
ISO = $(ISO_DIR)/ios.iso
LIMINE = Limine
LIMINE_BINARIES = limine-bios-cd.bin limine-uefi-cd.bin limine-bios.sys limine.conf
SRC := $(wildcard src/**/**/**/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/*.c) $(wildcard src/*.c) $(wildcard Flanterm/src/*.c) $(wildcard Flanterm/src/**/*.c)
#ASM_OBJECT = $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(filter %.s, $(SRC)))
ASM_SRC := $(wildcard src/**/**/**/*.s) $(wildcard src/**/**/*.s) $(wildcard src/**/*.s) $(wildcard src/*.s)
ASM_OBJECT = $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(ASM_SRC))

C_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))
OBJS := $(ASM_OBJECT) $(C_OBJS)

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
	qemu-system-x86_64 -cdrom $(ISO)

debug: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -s -S

clean:
	rm -r $(BUILD_DIR)/*
	rm -r $(ISO_DIR)/*
	