# Simple OS Makefile
# NOTE: All indented lines MUST use TAB characters, not spaces!

BOOT_ASM = boot.asm
KERNEL_C = kernel.c
VGA_C = vga.c
IDT_C = idt.c
KEYBOARD_C = keyboard.c
ISR_ASM = isr.asm
KUTILS_C = kutils.c
MUSIC_C = music.c
LINKER_SCRIPT = linker.ld

BOOT_BIN = boot.bin
KERNEL_BIN = kernel.bin
OS_IMAGE = os.img
VM_NAME = Kunix
BOOT_IMG = boot.img
DISK_SIZE = 100
DISK_IMAGE = kunix_disk.vdi

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

# Compile bootloader
$(BOOT_BIN): $(BOOT_ASM)
	$(NASM) -f bin $(BOOT_ASM) -o $(BOOT_BIN)

# Compile kernel
kernel.o: $(KERNEL_C)
	$(GCC) $(CFLAGS) $(KERNEL_C) -o kernel.o

# Compile vga
vga.o: $(VGA_C)
	$(GCC) $(CFLAGS) $(VGA_C) -o vga.o

# Compile idt
idt.o: $(IDT_C)
	$(GCC) $(CFLAGS) $(IDT_C) -o idt.o

# Assemble isr
isr.o: $(ISR_ASM)
	$(NASM) -f elf32 $(ISR_ASM) -o isr.o

keyboard.o: $(KEYBOARD_C)
	$(GCC) $(CFLAGS) $(KEYBOARD_C) -o keyboard.o

kutils.o: $(KUTILS_C)
	$(GCC) $(CFLAGS) $(KUTILS_C) -o kutils.o

music.o: $(MUSIC_C)
	$(GCC) $(CFLAGS) $(MUSIC_C) -o music.o

ata.o: ata.c
	$(GCC) $(CFLAGS) ata.c -o ata.o

# Link kernel
$(KERNEL_BIN): kernel.o vga.o idt.o isr.o keyboard.o kutils.o music.o ata.o $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) kernel.o vga.o idt.o isr.o keyboard.o kutils.o music.o ata.o -o $(KERNEL_BIN)

# Create OS image (bootloader + kernel)
$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	$(DD) if=/dev/zero of=$(OS_IMAGE) bs=1024 count=1440
	$(DD) if=$(BOOT_BIN) of=$(OS_IMAGE) conv=notrunc
	$(DD) if=$(KERNEL_BIN) of=$(OS_IMAGE) bs=512 seek=1 conv=notrunc

# VirtualBox setup with hard disk
# VirtualBox setup with hard disk
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
	$(VBOXMANAGE) storageattach $(VM_NAME) --storagectl "IDE Controller" --port 0 --device 0 --type hdd --medium $(shell pwd)/$(DISK_IMAGE) 2>/dev/null || truey

# Run in VirtualBox
run: setup-vm
	$(VBOXMANAGE) startvm $(VM_NAME)

# Clean build files
clean:
	rm -f $(BOOT_BIN) $(KERNEL_BIN) $(OS_IMAGE) $(BOOT_IMG) *.o

# Clean VM and disk
clean-vm:
	$(VBOXMANAGE) list vms | grep -q "$(VM_NAME)" && ( \
		$(VBOXMANAGE) controlvm $(VM_NAME) poweroff 2>/dev/null || true && \
		sleep 2 && \
		$(VBOXMANAGE) unregistervm $(VM_NAME) --delete ) || true
	rm -f $(DISK_IMAGE)

.PHONY: all run clean clean-vm setup-vm