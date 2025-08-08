# Simple OS Makefile
# NOTE: All indented lines MUST use TAB characters, not spaces!

BOOT_ASM = boot.asm
KERNEL_C = kernel.c
VGA_C = vga.c
IDT_C = idt.c
ISR_ASM = isr.asm
LINKER_SCRIPT = linker.ld

BOOT_BIN = boot.bin
KERNEL_BIN = kernel.bin
OS_IMAGE = os.img
VM_NAME = Kunix
BOOT_IMG = boot.img

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

# Link kernel
$(KERNEL_BIN): kernel.o vga.o idt.o isr.o $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) kernel.o vga.o idt.o isr.o -o $(KERNEL_BIN)

# Create OS image (bootloader + kernel)
$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	$(DD) if=/dev/zero of=$(OS_IMAGE) bs=1024 count=1440
	$(DD) if=$(BOOT_BIN) of=$(OS_IMAGE) conv=notrunc
	$(DD) if=$(KERNEL_BIN) of=$(OS_IMAGE) bs=512 seek=1 conv=notrunc

# VirtualBox setup
setup-vm: $(OS_IMAGE)
	$(VBOXMANAGE) list vms | grep -q "$(VM_NAME)" || ( \
		$(VBOXMANAGE) createvm --name $(VM_NAME) --register && \
		$(VBOXMANAGE) modifyvm $(VM_NAME) --memory 512 --boot1 floppy && \
		$(VBOXMANAGE) storagectl $(VM_NAME) --name "Floppy Controller" --add floppy )
	$(VBOXMANAGE) storageattach $(VM_NAME) --storagectl "Floppy Controller" --port 0 --device 0 --type fdd --medium $(shell pwd)/$(OS_IMAGE)

# Run in VirtualBox
run: setup-vm
	$(VBOXMANAGE) startvm $(VM_NAME)

# Clean build files
clean:
	rm -f $(BOOT_BIN) $(KERNEL_BIN) $(OS_IMAGE) $(BOOT_IMG) *.o

# Clean VM
clean-vm:
	$(VBOXMANAGE) list vms | grep -q "$(VM_NAME)" && ( \
		$(VBOXMANAGE) controlvm $(VM_NAME) poweroff 2>/dev/null || true && \
		sleep 2 && \
		$(VBOXMANAGE) unregistervm $(VM_NAME) --delete ) || true

.PHONY: all run clean clean-vm setup-vm