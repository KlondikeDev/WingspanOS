# WingspanOS

A lightweight, educational 32-bit operating system built from scratch.

## Features

- **Custom two-stage bootloader** - Handles larger kernels with proper CHS addressing
- **KLFS (Klondike Little Filesystem)** - Custom filesystem implementation
- **Interactive shell** with keyboard input
- **ATA driver** for disk I/O
- **PC speaker sound support**
- **Real-time clock integration**
- **Apache 2.0 licensed** - Free for educational and commercial use

## Quick Start

### Prerequisites
- GCC cross-compiler (i686-elf-gcc)
- NASM assembler
- Make

### Build & Run
```bash
make                    # Build OS image
make run                # Run in VirtualBox
qemu-system-i386 -fda os.img  # Run in QEMU
```

## Architecture

1. **Stage 1 Bootloader** (boot.asm) - BIOS loads this, loads Stage 2
2. **Stage 2 Bootloader** (stage2.asm) - Loads kernel, enters protected mode
3. **Kernel** (kernel.c) - Main OS with filesystem, drivers, and shell

## Purpose

WingspanOS provides a solid foundation for learning operating system development. The codebase is clean, well-commented, and designed to be extended by others interested in systems programming.
WingspanOS is designed to be expanded upon, and provides a minimal starting point for others to build on.

## License

Apache License 2.0 - Use freely with attribution required.

---

*Copyright (C) 2025 Joseph Jones (KlondikeDev)*
