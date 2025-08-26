# Two-Stage Bootloader Makefile
# NOTE: All indented lines MUST use TAB characters, not spaces!

# Source files
STAGE1_ASM = boot.asm
STAGE2_ASM = stage2.asm
KERNEL_C = kernel.c
VGA_C = vga.c
IDT_C = idt.c
KEYBOARD_C = keyboard.c
ISR_ASM = isr.asm
KUTILS_C = kutils.c
MUSIC_C = music.c
ATA_C = ata.c
KERNEL_ENTRY_ASM = kernel_entry.asm
LINKER_SCRIPT = linker.ld

# Output files
STAGE1_BIN = stage1.bin
STAGE2_BIN = stage2.bin
KERNEL_BIN = kernel.bin
OS_IMAGE = os.img
VM_NAME = Kunix
DISK_SIZE = 100
DISK_IMAGE = kunix_disk.vdi

# Configuration
STAGE2_SECTORS = 8
KERNEL_SECTORS = 64  # Increased from 36 - adjust as needed
# Standard 1.44MB floppy has 2880 sectors
FLOPPY_SECTORS = 2880

# Tools
NASM = nasm
GCC = gcc
LD = ld
DD = dd
VBOXMANAGE = VBoxManage

# Flags
CFLAGS = -m32 -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -fno-pic -fno-pie -Wall -Wextra -c
LDFLAGS = -m elf_i386 -T $(LINKER_SCRIPT) --oformat binary

all: $(OS_IMAGE)

# Stage 1 bootloader (512 bytes)
$(STAGE1_BIN): $(STAGE1_ASM)
	$(NASM) -f bin $(STAGE1_ASM) -o $(STAGE1_BIN)

# Stage 2 bootloader
$(STAGE2_BIN): $(STAGE2_ASM)
	$(NASM) -f bin $(STAGE2_ASM) -o $(STAGE2_BIN)
	# Pad Stage 2 to exactly STAGE2_SECTORS * 512 bytes
	@SIZE=$$(stat -c%s $(STAGE2_BIN) 2>/dev/null || stat -f%z $(STAGE2_BIN)); \
	NEEDED=$$(($(STAGE2_SECTORS) * 512)); \
	if [ $$SIZE -gt $$NEEDED ]; then \
		echo "Error: Stage 2 is $$SIZE bytes, max is $$NEEDED"; exit 1; \
	fi; \
	$(DD) if=/dev/zero bs=1 count=$$(($$NEEDED - $$SIZE)) >> $(STAGE2_BIN) 2>/dev/null

# Compile kernel objects
kernel.o: $(KERNEL_C)
	$(GCC) $(CFLAGS) $(KERNEL_C) -o kernel.o

vga.o: $(VGA_C)
	$(GCC) $(CFLAGS) $(VGA_C) -o vga.o

idt.o: $(IDT_C)
	$(GCC) $(CFLAGS) $(IDT_C) -o idt.o

isr.o: $(ISR_ASM)
	$(NASM) -f elf32 $(ISR_ASM) -o isr.o

keyboard.o: $(KEYBOARD_C)
	$(GCC) $(CFLAGS) $(KEYBOARD_C) -o keyboard.o

kutils.o: $(KUTILS_C)
	$(GCC) $(CFLAGS) $(KUTILS_C) -o kutils.o

music.o: $(MUSIC_C)
	$(GCC) $(CFLAGS) $(MUSIC_C) -o music.o

ata.o: $(ATA_C)
	$(GCC) $(CFLAGS) $(ATA_C) -o ata.o
# Add this rule after the other .o rules:
kernel_entry.o: $(KERNEL_ENTRY_ASM)
	$(NASM) -f elf32 $(KERNEL_ENTRY_ASM) -o kernel_entry.o

# Link kernel
$(KERNEL_BIN): kernel_entry.o kernel.o vga.o idt.o isr.o keyboard.o kutils.o music.o ata.o $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) kernel_entry.o kernel.o vga.o idt.o isr.o keyboard.o kutils.o music.o ata.o -o $(KERNEL_BIN)

# Create final OS image: Stage1 + Stage2 + Kernel (as a proper 1.44MB floppy)
$(OS_IMAGE): $(STAGE1_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	# Create standard 1.44MB floppy image
	$(DD) if=/dev/zero of=$(OS_IMAGE) bs=512 count=$(FLOPPY_SECTORS) 2>/dev/null
	
	# Copy Stage 1 (sector 0)
	$(DD) if=$(STAGE1_BIN) of=$(OS_IMAGE) bs=512 count=1 conv=notrunc 2>/dev/null
	
	# Copy Stage 2 (sectors 1 to STAGE2_SECTORS)
	$(DD) if=$(STAGE2_BIN) of=$(OS_IMAGE) bs=512 seek=1 count=$(STAGE2_SECTORS) conv=notrunc 2>/dev/null
	
	# Copy Kernel (starts after Stage 2)
	$(DD) if=$(KERNEL_BIN) of=$(OS_IMAGE) bs=512 seek=$$((1 + $(STAGE2_SECTORS))) conv=notrunc 2>/dev/null
	
	@echo "Image created (1.44MB floppy):"
	@echo "  Stage 1: Sector 0"
	@echo "  Stage 2: Sectors 1-$(STAGE2_SECTORS)"
	@echo "  Kernel:  Sectors $$(($(STAGE2_SECTORS) + 1)) onwards"

# VirtualBox setup
setup-vm: $(OS_IMAGE)
	$(VBOXMANAGE) list vms | grep -q "$(VM_NAME)" || ( \
		$(VBOXMANAGE) createvm --name $(VM_NAME) --register && \
		$(VBOXMANAGE) modifyvm $(VM_NAME) --memory 512 --boot1 floppy )
	
	# Check if controllers exist, create if not
	$(VBOXMANAGE) showvminfo $(VM_NAME) | grep -q "Floppy Controller" || \
		$(VBOXMANAGE) storagectl $(VM_NAME) --name "Floppy Controller" --add floppy
	
	$(VBOXMANAGE) showvminfo $(VM_NAME) | grep -q "IDE Controller" || \
		$(VBOXMANAGE) storagectl $(VM_NAME) --name "IDE Controller" --add ide
	
	# Create hard disk if it doesn't exist
	[ -f $(DISK_IMAGE) ] || $(VBOXMANAGE) createmedium disk --filename $(shell pwd)/$(DISK_IMAGE) --size $(DISK_SIZE)
	
	# Attach storage devices
	$(VBOXMANAGE) storageattach $(VM_NAME) --storagectl "Floppy Controller" --port 0 --device 0 --type fdd --medium $(shell pwd)/$(OS_IMAGE) 2>/dev/null || true
	$(VBOXMANAGE) storageattach $(VM_NAME) --storagectl "IDE Controller" --port 0 --device 0 --type hdd --medium $(shell pwd)/$(DISK_IMAGE) 2>/dev/null || true

# Run in VirtualBox
run: setup-vm
	$(VBOXMANAGE) startvm $(VM_NAME)

# Show disk layout
info:
	@echo "Disk Layout (1.44MB Floppy):"
	@echo "  Sector 0:     Stage 1 bootloader (512 bytes)"
	@echo "  Sectors 1-$(STAGE2_SECTORS):  Stage 2 bootloader ($$(($(STAGE2_SECTORS) * 512)) bytes)"
	@echo "  Sectors $$(($(STAGE2_SECTORS) + 1))+: Kernel (up to $$(($(KERNEL_SECTORS) * 512)) bytes)"
	@echo ""
	@echo "Total image size: 1.44MB ($(FLOPPY_SECTORS) sectors)"

# Clean build files
clean:
	rm -f $(STAGE1_BIN) $(STAGE2_BIN) $(KERNEL_BIN) $(OS_IMAGE) *.o

# Clean VM and disk
clean-vm:
	$(VBOXMANAGE) list vms | grep -q "$(VM_NAME)" && ( \
		$(VBOXMANAGE) controlvm $(VM_NAME) poweroff 2>/dev/null || true && \
		sleep 2 && \
		$(VBOXMANAGE) unregistervm $(VM_NAME) --delete ) || true
	rm -f $(DISK_IMAGE)

.PHONY: all run clean clean-vm setup-vm info