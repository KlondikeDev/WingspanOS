/*
 * WingspanOS v1.0 Kernel Core
 * Copyright 2025 Joseph Jones
 * Licensed under GPL v3
 *
 * File: kernel.c
 * Purpose: Main kernel initialization and event loop
 *
 * Refactored for v1.0 - Command handling moved to shell module,
 * better separation of concerns, reduced global state.
 */

#include "../../../include/types.h"
#include "../../../include/config.h"
#include "../../../include/vga.h"
#include "../../../include/idt.h"
#include "../../../include/kutils.h"
#include "../../../include/ata.h"
#include "../../../include/shell.h"

/* Input handling state */
typedef struct {
    char buffer[SHELL_INPUT_BUFFER_SIZE];
    u32 position;
    u16 start_row;
    u16 start_col;
    bool line_ready;
} InputContext;

static InputContext input_ctx = {0};

/* Forward declarations */
void kernel_init();
void kernel_print_banner();
void kernel_main_loop();

/* Main kernel entry point (called from kernel_entry.asm) */
void kmain() {
    kernel_init();
    kernel_print_banner();
    kernel_main_loop();
}

/* Initialize all kernel subsystems */
void kernel_init() {
    /* CRITICAL: Initialize VGA variables FIRST before any printing */
    row = 0;
    col = 0;

    /* Clear the screen */
    klear();

    /* Initialize interrupt descriptor table */
    kprint("Initializing IDT...\n");
    idt_init();

    /* Initialize storage subsystem */
    kprint("Detecting drives...\n");
    detect_drives();

    kprint("Kernel initialization complete.\n\n");
}

/* Print OS banner and copyright */
void kernel_print_banner() {
    kprint(OS_COPYRIGHT "\n");
    kprint(OS_LICENSE "\n\n");
    kprint("===========================================\n");
    kprint("     W I N G S P A N   O S   " OS_VERSION "         \n");
    kprint("===========================================\n");
}

/* Main kernel event loop */
void kernel_main_loop() {
    /* Print initial prompt */
    shell_print_prompt();
    update_cursor(row, col);
    input_ctx.start_row = row;
    input_ctx.start_col = col;

    /* Main loop - process keyboard input and execute commands */
    while (1) {
        if (input_ctx.line_ready) {
            /* Null-terminate the input buffer */
            input_ctx.buffer[input_ctx.position] = '\0';

            /* Move to new line */
            kprint("\n");

            /* Execute the command */
            shell_execute_command(input_ctx.buffer);

            /* Reset input state */
            input_ctx.position = 0;
            input_ctx.line_ready = false;

            /* Print new prompt */
            shell_print_prompt();
            input_ctx.start_row = row;
            input_ctx.start_col = col;
            update_cursor(row, col);
        }

        /* Halt CPU until next interrupt */
        __asm__ volatile ("hlt");
    }
}

/* Called by keyboard interrupt handler when line is complete */
void line_completed() {
    input_ctx.line_ready = true;
}

/* Accessor for input context (used by keyboard driver) */
InputContext* get_input_context() {
    return &input_ctx;
}
