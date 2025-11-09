/*
 * WingspanOS v1.0 Configuration Header
 * Copyright 2025 Joseph Jones
 * Licensed under GPL v3
 *
 * File: config.h
 * Purpose: Centralized configuration constants for WingspanOS
 *
 * This file contains all configurable constants that were previously
 * hardcoded throughout the codebase. Modify values here to adjust
 * system behavior.
 */

#ifndef CONFIG_H
#define CONFIG_H

/* ========== SYSTEM INFORMATION ========== */
#define OS_NAME "WingspanOS"
#define OS_VERSION "v1.0"
#define OS_TAGLINE "Freedom to Build"
#define OS_COPYRIGHT "Copyright (C) 2025 Joseph Jones (KlondikeDev)"
#define OS_LICENSE "Licensed under GPL v3"

/* ========== SHELL CONFIGURATION ========== */
#define SHELL_PROMPT "wingspan-$ "
#define SHELL_INPUT_BUFFER_SIZE 256
#define SHELL_COMMAND_BUFFER_SIZE 256

/* ========== VGA DISPLAY CONFIGURATION ========== */
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY_BASE 0xB8000
#define VGA_CRTC_COMMAND_PORT 0x3D4
#define VGA_CRTC_DATA_PORT 0x3D5

/* VGA Colors */
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GRAY 7
#define VGA_COLOR_DARK_GRAY 8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW 14
#define VGA_COLOR_WHITE 15

/* Default colors */
#define VGA_DEFAULT_FG VGA_COLOR_LIGHT_GRAY
#define VGA_DEFAULT_BG VGA_COLOR_BLACK

/* ========== FILESYSTEM CONFIGURATION ========== */
#define FS_MAGIC 0x4B4C4653  /* "KLFS" */
#define FS_MAX_FILES 20
#define FS_MAX_FILENAME_LENGTH 32
#define FS_BLOCK_SIZE 1024  /* 1KB blocks */
#define FS_SECTORS_PER_BLOCK 2  /* 512 bytes per sector */
#define FS_TOTAL_BLOCKS 1440  /* For 1.44MB floppy */
#define FS_SUPERBLOCK_SECTOR 0

/* ========== ATA DRIVER CONFIGURATION ========== */
#define ATA_PRIMARY_BASE 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

/* ATA Ports (relative to base) */
#define ATA_DATA_PORT(base) ((base) + 0)
#define ATA_ERROR_PORT(base) ((base) + 1)
#define ATA_FEATURES_PORT(base) ((base) + 1)
#define ATA_SECTOR_COUNT_PORT(base) ((base) + 2)
#define ATA_LBA_LOW_PORT(base) ((base) + 3)
#define ATA_LBA_MID_PORT(base) ((base) + 4)
#define ATA_LBA_HIGH_PORT(base) ((base) + 5)
#define ATA_DRIVE_SELECT_PORT(base) ((base) + 6)
#define ATA_COMMAND_PORT(base) ((base) + 7)
#define ATA_STATUS_PORT(base) ((base) + 7)

/* ATA Commands */
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY 0xEC

/* ATA Status Bits */
#define ATA_STATUS_BSY 0x80  /* Busy */
#define ATA_STATUS_DRDY 0x40  /* Drive Ready */
#define ATA_STATUS_DRQ 0x08  /* Data Request */
#define ATA_STATUS_ERR 0x01  /* Error */

/* ATA Drive Select */
#define ATA_DRIVE_MASTER 0xA0
#define ATA_DRIVE_SLAVE 0xB0

/* ATA Timeouts */
#define ATA_TIMEOUT_ITERATIONS 10000

/* ========== INTERRUPT CONFIGURATION ========== */
#define IDT_ENTRIES 256

/* PIC (Programmable Interrupt Controller) */
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

/* IRQ Base (after remapping) */
#define IRQ_BASE 32

/* ========== KEYBOARD CONFIGURATION ========== */
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_BUFFER_SIZE 256

/* ========== SOUND CONFIGURATION ========== */
#define SOUND_PIT_CHANNEL2 0x42
#define SOUND_PIT_COMMAND 0x43
#define SOUND_PC_SPEAKER_PORT 0x61
#define SOUND_PIT_FREQUENCY 1193180  /* Hz */

/* Note durations (in iterations, crude timing) */
#define SOUND_QUARTER_NOTE 500000
#define SOUND_HALF_NOTE 1000000

/* ========== RTC CONFIGURATION ========== */
#define RTC_COMMAND_PORT 0x70
#define RTC_DATA_PORT 0x71

/* RTC Registers */
#define RTC_REG_SECONDS 0x00
#define RTC_REG_MINUTES 0x02
#define RTC_REG_HOURS 0x04
#define RTC_REG_DAY 0x07
#define RTC_REG_MONTH 0x08
#define RTC_REG_YEAR 0x09

/* ========== BOOT CONFIGURATION ========== */
#define BOOT_STAGE2_SECTOR 2
#define BOOT_STAGE2_LOAD_ADDRESS 0x7E00
#define BOOT_KERNEL_SECTOR 10
#define BOOT_KERNEL_LOAD_ADDRESS 0x10000
#define BOOT_KERNEL_SECTOR_COUNT 50  /* Adjust based on kernel size */

/* ========== MEMORY CONFIGURATION ========== */
#define KERNEL_STACK_SIZE (16 * 1024)  /* 16KB stack */
#define KERNEL_HEAP_SIZE (64 * 1024)  /* 64KB heap (future use) */

/* ========== ERROR CODES ========== */
#define ERROR_SUCCESS 0
#define ERROR_INVALID_ARGUMENT 1
#define ERROR_FILE_NOT_FOUND 2
#define ERROR_DISK_FULL 3
#define ERROR_FILE_EXISTS 4
#define ERROR_IO_ERROR 5
#define ERROR_NO_FILESYSTEM 6
#define ERROR_BUFFER_OVERFLOW 7
#define ERROR_OUT_OF_MEMORY 8

/* ========== DEBUG CONFIGURATION ========== */
#define DEBUG_ENABLED 0  /* Set to 1 to enable debug output */
#define VERBOSE_BOOT 0   /* Set to 1 for verbose boot messages */

#endif /* CONFIG_H */
