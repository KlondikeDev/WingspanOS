# boot/Kernel Makefile for VirtualBox
# NOTE: All indented lines MUST use TAB characters, not spaces!

ASM_FILE = boot.asm
BIN_FILE = boot.bin
IMG_FILE = boot.img
VM_NAME = bootVM

KERNEL_C_SOURCES = $(wildcard *.c)
KERNEL_OBJECTS = $(KERNEL_C_SOURCES:.c=.o) kernel.o
KERNEL_BIN = kernel.bin

NASM = nasm
GCC = gcc
LD = ld
VBOXMANAGE = VBoxManage
DD = dd

CFLAGS = -m32 -fno-builtin -fno-stack-protector -nostdlib -nodefaultlibs -Wall -Wextra -Werror -c
LDFLAGS = -m elf_i386 -Ttext 0x1000 --oformat binary
ASMFLAGS = -f elf32

all: run

linker.ld:
	echo "ENTRY(_start)" > linker.ld
	echo "SECTIONS {" >> linker.ld
	echo "  . = 0x1000;" >> linker.ld
	echo "  .text : { *(.text) }" >> linker.ld
	echo "  .data : { *(.data) }" >> linker.ld
	echo "  .bss : { *(.bss) }" >> linker.ld
	echo "}" >> linker.ld

$(BIN_FILE): $(ASM_FILE)
	$(NASM) -f bin $(ASM_FILE) -o $(BIN_FILE)

%.o: %.c
	$(GCC) $(CFLAGS) $< -o $@

kernel.o: kernel.asm
	$(NASM) $(ASMFLAGS) kernel.asm -o kernel.o

$(KERNEL_BIN): $(KERNEL_OBJECTS)
	$(LD) $(LDFLAGS) $(KERNEL_OBJECTS) -o $@

$(IMG_FILE): $(BIN_FILE)
	$(DD) if=/dev/zero of=$(IMG_FILE) bs=1024 count=1440
	$(DD) if=$(BIN_FILE) of=$(IMG_FILE) conv=notrunc

disk-with-kernel: $(BIN_FILE) $(KERNEL_BIN)
	$(DD) if=/dev/zero of=$(IMG_FILE) bs=1024 count=1440
	$(DD) if=$(BIN_FILE) of=$(IMG_FILE) conv=notrunc
	$(DD) if=$(KERNEL_BIN) of=$(IMG_FILE) bs=512 seek=1 conv=notrunc

setup-vm: $(IMG_FILE)
	$(VBOXMANAGE) list vms | grep -q "$(VM_NAME)" || ( \
		$(VBOXMANAGE) createvm --name $(VM_NAME) --register && \
		$(VBOXMANAGE) modifyvm $(VM_NAME) --memory 512 --boot1 floppy && \
		$(VBOXMANAGE) storagectl $(VM_NAME) --name "Floppy Controller" --add floppy )
	$(VBOXMANAGE) storageattach $(VM_NAME) --storagectl "Floppy Controller" --port 0 --device 0 --type fdd --medium $(shell pwd)/$(IMG_FILE)

run: setup-vm
	$(VBOXMANAGE) startvm $(VM_NAME)

run-kernel: disk-with-kernel setup-vm
	$(VBOXMANAGE) startvm $(VM_NAME)

kernel: $(KERNEL_BIN)

clean:
	rm -f $(BIN_FILE) $(IMG_FILE) $(KERNEL_BIN) *.o linker.ld

clean-vm:
	$(VBOXMANAGE) list vms | grep -q "$(VM_NAME)" && ( \
		$(VBOXMANAGE) controlvm $(VM_NAME) poweroff 2>/dev/null || true && \
		sleep 2 && \
		$(VBOXMANAGE) unregistervm $(VM_NAME) --delete ) || true

.PHONY: all run run-kernel kernel clean clean-vm setup-vm disk-with-kernel