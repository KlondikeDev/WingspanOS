#
# WingspanOS v1.0 Makefile
# Copyright 2025 Joseph Jones
# Licensed under GPL v3
#
# Purpose: Build system for WingspanOS with modular directory structure
#
# NOTE: All indented lines MUST use TAB characters, not spaces!

# Directory structure
SRC_DIR = src
BOOT_DIR = $(SRC_DIR)/boot
KERNEL_DIR = $(SRC_DIR)/kernel
DRIVERS_DIR = $(SRC_DIR)/drivers
LIB_DIR = $(SRC_DIR)/lib
INCLUDE_DIR = include
BUILD_DIR = build

# Source files (new structure)
STAGE1_ASM = $(BOOT_DIR)/boot.asm
STAGE2_ASM = $(BOOT_DIR)/stage2.asm
STAGE2_LD = $(BOOT_DIR)/stage2.ld

KERNEL_ENTRY_ASM = $(KERNEL_DIR)/init/kernel_entry.asm
KERNEL_C = $(KERNEL_DIR)/core/kernel_new.c
SHELL_C = $(KERNEL_DIR)/shell/shell.c
LINKER_SCRIPT = $(KERNEL_DIR)/linker.ld

VGA_C = $(DRIVERS_DIR)/video/vga.c
KEYBOARD_C = $(DRIVERS_DIR)/input/keyboard_new.c
ATA_C = $(DRIVERS_DIR)/storage/ata.c
MUSIC_C = $(DRIVERS_DIR)/sound/music.c
IDT_C = $(DRIVERS_DIR)/interrupt/idt.c
ISR_ASM = $(DRIVERS_DIR)/interrupt/isr.asm

KUTILS_C = $(LIB_DIR)/kutils.c

# Output files
STAGE1_BIN = $(BUILD_DIR)/stage1.bin
STAGE2_BIN = $(BUILD_DIR)/stage2.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
OS_IMAGE = os.img

# VirtualBox configuration
VM_NAME = WingspanOS
DISK_SIZE = 100
DISK_IMAGE = wingspan_disk.vdi

# Configuration
STAGE2_SECTORS = 8
KERNEL_SECTORS = 64
FLOPPY_SECTORS = 2880  # 1.44MB floppy

# Tools
NASM = nasm
GCC = gcc
LD = ld
DD = dd
VBOXMANAGE = VBoxManage

# Compiler flags
CFLAGS = -m32 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib \
         -fno-pic -fno-pie -Wall -Wextra -c -I$(INCLUDE_DIR)
LDFLAGS = -m elf_i386 -T $(LINKER_SCRIPT) --oformat binary

# Object files (in build directory)
OBJS = $(BUILD_DIR)/kernel_entry.o \
       $(BUILD_DIR)/kernel.o \
       $(BUILD_DIR)/shell.o \
       $(BUILD_DIR)/vga.o \
       $(BUILD_DIR)/idt.o \
       $(BUILD_DIR)/isr.o \
       $(BUILD_DIR)/keyboard.o \
       $(BUILD_DIR)/kutils.o \
       $(BUILD_DIR)/music.o \
       $(BUILD_DIR)/ata.o

.PHONY: all clean clean-vm setup-vm run info dirs

all: dirs $(OS_IMAGE)

# Create build directory
dirs:
	@mkdir -p $(BUILD_DIR)

# Stage 1 bootloader (512 bytes)
$(STAGE1_BIN): $(STAGE1_ASM) | dirs
	$(NASM) -f bin $(STAGE1_ASM) -o $(STAGE1_BIN)

# Stage 2 bootloader
$(STAGE2_BIN): $(STAGE2_ASM) | dirs
	$(NASM) -f bin $(STAGE2_ASM) -o $(STAGE2_BIN)
	@SIZE=$$(stat -c%s $(STAGE2_BIN) 2>/dev/null || stat -f%z $(STAGE2_BIN)); \
	NEEDED=$$(($(STAGE2_SECTORS) * 512)); \
	if [ $$SIZE -gt $$NEEDED ]; then \
		echo "Error: Stage 2 is $$SIZE bytes, max is $$NEEDED"; exit 1; \
	fi; \
	$(DD) if=/dev/zero bs=1 count=$$(($$NEEDED - $$SIZE)) >> $(STAGE2_BIN) 2>/dev/null

# Kernel entry (assembly)
$(BUILD_DIR)/kernel_entry.o: $(KERNEL_ENTRY_ASM) | dirs
	$(NASM) -f elf32 $(KERNEL_ENTRY_ASM) -o $(BUILD_DIR)/kernel_entry.o

# Kernel core
$(BUILD_DIR)/kernel.o: $(KERNEL_C) | dirs
	$(GCC) $(CFLAGS) $(KERNEL_C) -o $(BUILD_DIR)/kernel.o

# Shell
$(BUILD_DIR)/shell.o: $(SHELL_C) | dirs
	$(GCC) $(CFLAGS) $(SHELL_C) -o $(BUILD_DIR)/shell.o

# VGA driver
$(BUILD_DIR)/vga.o: $(VGA_C) | dirs
	$(GCC) $(CFLAGS) $(VGA_C) -o $(BUILD_DIR)/vga.o

# IDT driver
$(BUILD_DIR)/idt.o: $(IDT_C) | dirs
	$(GCC) $(CFLAGS) $(IDT_C) -o $(BUILD_DIR)/idt.o

# ISR (assembly)
$(BUILD_DIR)/isr.o: $(ISR_ASM) | dirs
	$(NASM) -f elf32 $(ISR_ASM) -o $(BUILD_DIR)/isr.o

# Keyboard driver
$(BUILD_DIR)/keyboard.o: $(KEYBOARD_C) | dirs
	$(GCC) $(CFLAGS) $(KEYBOARD_C) -o $(BUILD_DIR)/keyboard.o

# Kernel utilities
$(BUILD_DIR)/kutils.o: $(KUTILS_C) | dirs
	$(GCC) $(CFLAGS) $(KUTILS_C) -o $(BUILD_DIR)/kutils.o

# Music driver
$(BUILD_DIR)/music.o: $(MUSIC_C) | dirs
	$(GCC) $(CFLAGS) $(MUSIC_C) -o $(BUILD_DIR)/music.o

# ATA/Filesystem driver
$(BUILD_DIR)/ata.o: $(ATA_C) | dirs
	$(GCC) $(CFLAGS) $(ATA_C) -o $(BUILD_DIR)/ata.o

# Link kernel
$(KERNEL_BIN): $(OBJS) $(LINKER_SCRIPT) | dirs
	$(LD) $(LDFLAGS) $(OBJS) -o $(KERNEL_BIN)
	@echo "Kernel size: $$(stat -c%s $(KERNEL_BIN) 2>/dev/null || stat -f%z $(KERNEL_BIN)) bytes"

# Create final OS image
$(OS_IMAGE): $(STAGE1_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	$(DD) if=/dev/zero of=$(OS_IMAGE) bs=512 count=$(FLOPPY_SECTORS) 2>/dev/null
	$(DD) if=$(STAGE1_BIN) of=$(OS_IMAGE) bs=512 count=1 conv=notrunc 2>/dev/null
	$(DD) if=$(STAGE2_BIN) of=$(OS_IMAGE) bs=512 seek=1 count=$(STAGE2_SECTORS) conv=notrunc 2>/dev/null
	$(DD) if=$(KERNEL_BIN) of=$(OS_IMAGE) bs=512 seek=$$((1 + $(STAGE2_SECTORS))) conv=notrunc 2>/dev/null
	@echo "========================================="
	@echo "WingspanOS v1.0 image created!"
	@echo "========================================="
	@echo "  Stage 1: Sector 0"
	@echo "  Stage 2: Sectors 1-$(STAGE2_SECTORS)"
	@echo "  Kernel:  Sectors $$(($(STAGE2_SECTORS) + 1)) onwards"
	@echo "  Total:   1.44MB ($(FLOPPY_SECTORS) sectors)"
	@echo "========================================="

# VirtualBox setup
setup-vm: $(OS_IMAGE)
	$(VBOXMANAGE) list vms | grep -q "$(VM_NAME)" || ( \
		$(VBOXMANAGE) createvm --name $(VM_NAME) --register && \
		$(VBOXMANAGE) modifyvm $(VM_NAME) --memory 512 --boot1 floppy )
	$(VBOXMANAGE) showvminfo $(VM_NAME) | grep -q "Floppy Controller" || \
		$(VBOXMANAGE) storagectl $(VM_NAME) --name "Floppy Controller" --add floppy
	$(VBOXMANAGE) showvminfo $(VM_NAME) | grep -q "IDE Controller" || \
		$(VBOXMANAGE) storagectl $(VM_NAME) --name "IDE Controller" --add ide
	[ -f $(DISK_IMAGE) ] || $(VBOXMANAGE) createmedium disk --filename $(shell pwd)/$(DISK_IMAGE) --size $(DISK_SIZE)
	$(VBOXMANAGE) storageattach $(VM_NAME) --storagectl "Floppy Controller" --port 0 --device 0 --type fdd --medium $(shell pwd)/$(OS_IMAGE) 2>/dev/null || true
	$(VBOXMANAGE) storageattach $(VM_NAME) --storagectl "IDE Controller" --port 0 --device 0 --type hdd --medium $(shell pwd)/$(DISK_IMAGE) 2>/dev/null || true

# Run in VirtualBox
run: setup-vm
	$(VBOXMANAGE) startvm $(VM_NAME)

# Show disk layout
info:
	@echo "WingspanOS v1.0 Build Information"
	@echo "=================================="
	@echo "Disk Layout (1.44MB Floppy):"
	@echo "  Sector 0:       Stage 1 bootloader (512 bytes)"
	@echo "  Sectors 1-$(STAGE2_SECTORS):    Stage 2 bootloader ($$(($(STAGE2_SECTORS) * 512)) bytes)"
	@echo "  Sectors $$(($(STAGE2_SECTORS) + 1))+:   Kernel (up to $$(($(KERNEL_SECTORS) * 512)) bytes)"
	@echo ""
	@echo "Directory Structure:"
	@echo "  $(SRC_DIR)/boot/        - Bootloader"
	@echo "  $(SRC_DIR)/kernel/      - Kernel core and shell"
	@echo "  $(SRC_DIR)/drivers/     - Device drivers"
	@echo "  $(SRC_DIR)/lib/         - Utilities"
	@echo "  $(INCLUDE_DIR)/         - Headers"
	@echo "  $(BUILD_DIR)/           - Build artifacts"

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(OS_IMAGE)

# Clean VM and disk
clean-vm:
	$(VBOXMANAGE) list vms | grep -q "$(VM_NAME)" && ( \
		$(VBOXMANAGE) controlvm $(VM_NAME) poweroff 2>/dev/null || true && \
		sleep 2 && \
		$(VBOXMANAGE) unregistervm $(VM_NAME) --delete ) || true
	rm -f $(DISK_IMAGE)

# Full clean
distclean: clean clean-vm
	@echo "All build artifacts and VM cleaned."
