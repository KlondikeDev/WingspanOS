/*
 * Copyright 2025 Joseph Jones
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
    File: vga.c
    Created on: August 8th 2025
    Created by: jjones (GitHub Username: KlondikeDev)
    Purpose: Adds vga text printing functionality.
    Dependencies: types.h, vga.h, idt.h

    Suggested Changes/Todo:
    Nothing to do.

*/

#include "types.h"
#include "vga.h"
#include "idt.h"
#include "kutils.h"

u16 row = 0, col = 0;
u16 input_start_row = 0;
u16 input_start_col = 0;

void scroll_screen() {
    u16* VGA_MEMORY = (u16*) (0xB8000);
    
    // Move each line up by one
    for(int line = 0; line < 24; line++) {
        for(int col = 0; col < 80; col++) {
            VGA_MEMORY[line * 80 + col] = VGA_MEMORY[(line + 1) * 80 + col];
        }
    }
    
    // Clear the bottom line
    for(int col = 0; col < 80; col++) {
        VGA_MEMORY[24 * 80 + col] = (VGA_COLOR(VGA_BLACK, VGA_WHITE) << 8) | ' ';
    }
}

void kprint(const char* str){
    u16* VGA_MEMORY = (u16*) (0xB8000);
    
    for(u32 i = 0; str[i] != '\0'; i++){
        if(str[i] == '\n'){
            row++;
            col = 0;
            update_cursor(row, col);
        } else{
            VGA_MEMORY[row * 80 + col] = (str[i]) | (VGA_COLOR(VGA_BLACK, VGA_WHITE) << 8);
            col++;
            if(col >= 80) {  // Wrap to next line
                row++;
                col = 0;
                update_cursor(row, col);
            }
        }

        if(row >= 25) {
            // Time to scroll!
            scroll_screen();
            row = 24;  // Move to last line
            col = 0;
            update_cursor(row, col);
        }
        update_cursor(row, col);  // <-- ADD THIS
    }
}
void klear(){
    u16* VGA_MEMORY = (u16*) (0xB8000);
    for(u32 i = 0; i < 80 * 25; i++){
        VGA_MEMORY[i] = (VGA_COLOR(VGA_BLACK, VGA_WHITE) << 8) | ' ';
    }
}

void kprint_isr(const char* str) {
    u16* VGA_MEMORY = (u16*) (0xB8000);  // Local instead of global
    static u16 isr_cursor = 80 * 10;     // Start on line 10 for errors
    
    for(u32 i = 0; str[i] != '\0'; i++){
        if(str[i] == '\n'){
            isr_cursor = (isr_cursor / 80 + 1) * 80;
        } else{
            VGA_MEMORY[isr_cursor] = (str[i]) | (VGA_COLOR(VGA_RED, VGA_WHITE) << 8);
            isr_cursor++;
        }

        if(isr_cursor >= 80 * 25)
            isr_cursor = 80 * 10;  // Wrap back to line 10
    }
}

// Add to vga.c  
void kprint_hex(u8 value) {
    char buffer[5] = "0x00";
    char hex_chars[] = "0123456789ABCDEF";
    
    buffer[2] = hex_chars[(value >> 4) & 0xF];  // Upper 4 bits
    buffer[3] = hex_chars[value & 0xF];         // Lower 4 bits
    
    kprint(buffer);
}

void kprint_dec(u32 num) {
    char buffer[12];  // Enough for 32-bit number
    int i = 0;

    if(num == 0) {
        kprint("0");
        return;
    }

    while(num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    // Print in reverse
    while(--i >= 0) {
        char str[2] = {buffer[i], '\0'};
        kprint(str);
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