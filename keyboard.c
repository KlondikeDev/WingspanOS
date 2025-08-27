/*
    File: keyboard.c
    Created on: August 8th 2025
    Created by: jjones (GitHub Username: KlondikeDev)
    Purpose: To setup keyboard input handling.
    Dependencies: types.h, vga.h, idt.h, kutils.h

    Suggested Changes/Todo:
    Nothing to do.

*/


#include "types.h"
#include "idt.h"
#include "vga.h"
#include "kutils.h"
bool shiftDown = false;


extern u32 input_pos;
extern void line_completed();
extern char input_buffer[];  // Remove the = 0 initialization
extern u32 input_pos;        // Remove the = 0 initialization  
extern bool line_ready;      // Remove the = false initialization

char kbdus[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', '-', '=', '\b',   /* Backspace */
    '\t',           /* Tab */
    'q', 'w', 'e', 'r', /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',       /* Enter key */
    0,          /* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',   /* 39 */
    '\'', '`',   0,     /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',         /* 49 */
    'm', ',', '.', '/',   0,                    /* Right shift */
    '*',
    0,  /* Alt */
    ' ',    /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

char kbdusShifted[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
    '(', ')', '_', '+', '\b', /* Backspace */
    '\t', /* Tab */
    'Q', 'W', 'E', 'R', /* 19 */
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */
    0, /* 29 - Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
    '"', '~', 0, /* Left shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', /* 49 */
    'M', '<', '>', '?', 0, /* Right shift */
    '*',
    0, /* Alt */
    ' ', /* Space bar */
    0, /* Caps lock */
    0, /* 59 - F1 key ... > */
    0, 0, 0, 0, 0, 0, 0, 0,
    0, /* < ... F10 */
    0, /* 69 - Num lock*/
    0, /* Scroll Lock */
    0, /* Home key */
    0, /* Up Arrow */
    0, /* Page Up */
    '-',
    0, /* Left Arrow */
    0,
    0, /* Right Arrow */
    '+',
    0, /* 79 - End key*/
    0, /* Down Arrow */
    0, /* Page Down */
    0, /* Insert Key */
    0, /* Delete Key */
    0, 0, 0,
    0, /* F11 Key */
    0, /* F12 Key */
    0, /* All other keys are undefined */
};


void handle_backspace(void){
    // Don't backspace past where input started
    if(row < input_start_row || (row == input_start_row && col <= input_start_col)) {
        return;
    }

    input_pos--;
    
    if(col > 0) {
        col--;
    } else if(row > 0) {
        row--;
        col = 79;
    }
    
    u16* VGA_MEMORY = (u16*) (0xB8000);
    VGA_MEMORY[row * 80 + col] = (VGA_COLOR(VGA_BLACK, VGA_WHITE) << 8) | ' ';
}


void read_key_from_port(void) {
    u8 scancode = inb(0x60);
    
    // Handle shift press/release first
    if (scancode == 0x2A) {
        shiftDown = true;
    } else if (scancode == 0xAA) {
        shiftDown = false;
    }
    // Key press only (not release) for other keys
    else if (!(scancode & 0x80)) {
        if (scancode == 0x0E) {
            handle_backspace();
            update_cursor(row, col);
        } else if (scancode == 0x1C) { // Enter key
            kprint("\n");
            update_cursor(row, col);
            input_start_row = row;
            input_start_col = col;
            input_buffer[input_pos] = '\0'; // Null terminate
            line_completed(); // Tell kernel line is ready
        } else {
            if (shiftDown == false) {
                char key = kbdus[scancode];
                if (key != 0 && input_pos < 255) {  // ADD BOUNDS CHECK HERE
                    input_buffer[input_pos++] = key;
                    char str[2] = {key, '\0'};
                    kprint(str); // Still echo to screen
                }
            } else {
                char key = kbdusShifted[scancode];
                if (key != 0 && input_pos < 255) {  // ADD BOUNDS CHECK HERE
                    input_buffer[input_pos++] = key;
                    char str[2] = {key, '\0'};
                    kprint(str); // Still echo to screen
                }
            }
        }
    }
}
void update_cursor(u16 row, u16 col) {
    u16 pos = row * 80 + col;
    
    // Send high byte
    outb(0x3D4, 14);
    outb(0x3D5, (pos >> 8) & 0xFF);
    
    // Send low byte  
    outb(0x3D4, 15);
    outb(0x3D5, pos & 0xFF);
}