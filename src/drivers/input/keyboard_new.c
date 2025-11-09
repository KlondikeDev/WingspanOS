/*
 * WingspanOS v1.0 Keyboard Driver
 * Copyright 2025 Joseph Jones
 * Licensed under GPL v3
 *
 * File: keyboard.c
 * Purpose: PS/2 keyboard input handling
 *
 * Refactored for v1.0:
 * - Eliminated duplicate keyboard mapping arrays
 * - Uses config.h constants
 * - Works with input context structure instead of globals
 * - Better scancode handling
 */

#include "../../../include/types.h"
#include "../../../include/config.h"
#include "../../../include/idt.h"
#include "../../../include/vga.h"
#include "../../../include/kutils.h"

/* Forward declaration of input context */
typedef struct {
    char buffer[SHELL_INPUT_BUFFER_SIZE];
    u32 position;
    u16 start_row;
    u16 start_col;
    bool line_ready;
} InputContext;

extern InputContext* get_input_context();
extern void line_completed();

/* Keyboard state */
static bool shift_pressed = false;

/* Scancode definitions */
#define SCANCODE_BACKSPACE 0x0E
#define SCANCODE_ENTER 0x1C
#define SCANCODE_LSHIFT_PRESS 0x2A
#define SCANCODE_RSHIFT_PRESS 0x36
#define SCANCODE_LSHIFT_RELEASE 0xAA
#define SCANCODE_RSHIFT_RELEASE 0xB6
#define SCANCODE_RELEASE_FLAG 0x80

/*
 * Unified keyboard scancode mapping
 * Each entry is: {normal_char, shifted_char}
 */
typedef struct {
    char normal;
    char shifted;
} KeyMapping;

static const KeyMapping scancode_map[128] = {
    [0x00] = {0, 0},
    [0x01] = {27, 27},      /* Escape */
    [0x02] = {'1', '!'},
    [0x03] = {'2', '@'},
    [0x04] = {'3', '#'},
    [0x05] = {'4', '$'},
    [0x06] = {'5', '%'},
    [0x07] = {'6', '^'},
    [0x08] = {'7', '&'},
    [0x09] = {'8', '*'},
    [0x0A] = {'9', '('},
    [0x0B] = {'0', ')'},
    [0x0C] = {'-', '_'},
    [0x0D] = {'=', '+'},
    [0x0E] = {'\b', '\b'},  /* Backspace */
    [0x0F] = {'\t', '\t'},  /* Tab */
    [0x10] = {'q', 'Q'},
    [0x11] = {'w', 'W'},
    [0x12] = {'e', 'E'},
    [0x13] = {'r', 'R'},
    [0x14] = {'t', 'T'},
    [0x15] = {'y', 'Y'},
    [0x16] = {'u', 'U'},
    [0x17] = {'i', 'I'},
    [0x18] = {'o', 'O'},
    [0x19] = {'p', 'P'},
    [0x1A] = {'[', '{'},
    [0x1B] = {']', '}'},
    [0x1C] = {'\n', '\n'},  /* Enter */
    [0x1D] = {0, 0},        /* Control */
    [0x1E] = {'a', 'A'},
    [0x1F] = {'s', 'S'},
    [0x20] = {'d', 'D'},
    [0x21] = {'f', 'F'},
    [0x22] = {'g', 'G'},
    [0x23] = {'h', 'H'},
    [0x24] = {'j', 'J'},
    [0x25] = {'k', 'K'},
    [0x26] = {'l', 'L'},
    [0x27] = {';', ':'},
    [0x28] = {'\'', '"'},
    [0x29] = {'`', '~'},
    [0x2A] = {0, 0},        /* Left Shift */
    [0x2B] = {'\\', '|'},
    [0x2C] = {'z', 'Z'},
    [0x2D] = {'x', 'X'},
    [0x2E] = {'c', 'C'},
    [0x2F] = {'v', 'V'},
    [0x30] = {'b', 'B'},
    [0x31] = {'n', 'N'},
    [0x32] = {'m', 'M'},
    [0x33] = {',', '<'},
    [0x34] = {'.', '>'},
    [0x35] = {'/', '?'},
    [0x36] = {0, 0},        /* Right Shift */
    [0x37] = {'*', '*'},    /* Keypad * */
    [0x38] = {0, 0},        /* Alt */
    [0x39] = {' ', ' '},    /* Space */
    /* All other scancodes default to {0, 0} */
};

/* Handle backspace key */
static void handle_backspace() {
    InputContext* ctx = get_input_context();

    /* Don't backspace past where input started */
    if (row < ctx->start_row || (row == ctx->start_row && col <= ctx->start_col)) {
        return;
    }

    /* Only decrement position if we have characters */
    if (ctx->position > 0) {
        ctx->position--;
    }

    /* Update cursor position */
    if (col > 0) {
        col--;
    } else if (row > 0) {
        row--;
        col = VGA_WIDTH - 1;
    }

    /* Clear the character at cursor position */
    u16* vga_mem = (u16*)(VGA_MEMORY_BASE);
    vga_mem[row * VGA_WIDTH + col] = (VGA_COLOR(VGA_DEFAULT_BG, VGA_DEFAULT_FG) << 8) | ' ';
}

/* Handle enter key */
static void handle_enter(InputContext* ctx) {
    /* Don't process if line is already ready */
    if (ctx->line_ready) {
        return;
    }

    /* Null-terminate the buffer */
    ctx->buffer[ctx->position] = '\0';

    /* Signal that line is complete */
    line_completed();
}

/* Handle printable character */
static void handle_character(char ch, InputContext* ctx) {
    /* Check buffer bounds */
    if (ctx->position >= SHELL_INPUT_BUFFER_SIZE - 1) {
        return;  /* Buffer full */
    }

    /* Add to buffer */
    ctx->buffer[ctx->position++] = ch;

    /* Echo to screen */
    char str[2] = {ch, '\0'};
    kprint(str);
}

/* Main keyboard interrupt handler */
void read_key_from_port() {
    InputContext* ctx = get_input_context();
    u8 scancode = inb(KEYBOARD_DATA_PORT);

    /* Handle shift key state */
    if (scancode == SCANCODE_LSHIFT_PRESS || scancode == SCANCODE_RSHIFT_PRESS) {
        shift_pressed = true;
        return;
    }
    else if (scancode == SCANCODE_LSHIFT_RELEASE || scancode == SCANCODE_RSHIFT_RELEASE) {
        shift_pressed = false;
        return;
    }

    /* Ignore key releases (high bit set) */
    if (scancode & SCANCODE_RELEASE_FLAG) {
        return;
    }

    /* Handle special keys */
    if (scancode == SCANCODE_BACKSPACE) {
        handle_backspace();
        update_cursor(row, col);
        return;
    }

    if (scancode == SCANCODE_ENTER) {
        handle_enter(ctx);
        return;
    }

    /* Look up character in scancode map */
    if (scancode < 128) {
        char ch = shift_pressed ? scancode_map[scancode].shifted
                                : scancode_map[scancode].normal;

        if (ch != 0) {
            handle_character(ch, ctx);
        }
    }
}
