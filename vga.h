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
    File: vga.h
    Created on: August 9th 2025
    Created by: jjones (GitHub Username: KlondikeDev)
    Purpose: Header file for vga text printing functionality.
    Dependencies: types.h, vga.h, idt.h

    Suggested Changes/Todo:
    Nothing to do.

*/

#ifndef VGA_H
#define VGA_H
#include "types.h"
typedef enum {
    VGA_BLACK = 0,
    VGA_BLUE = 1,
    VGA_GREEN = 2,
    VGA_CYAN = 3,
    VGA_RED = 4,
    VGA_MAGENTA = 5,
    VGA_BROWN = 6,
    VGA_LIGHT_GRAY = 7,
    VGA_DARK_GRAY = 8,
    VGA_LIGHT_BLUE = 9,
    VGA_LIGHT_GREEN = 10,
    VGA_LIGHT_CYAN = 11,
    VGA_LIGHT_RED = 12,
    VGA_LIGHT_MAGENTA = 13,
    VGA_YELLOW = 14,
    VGA_WHITE = 15
} vga_color_t;

#define VGA_COLOR(bg, fg) ((bg << 4) | fg)

void kprint(const char* str);
void klear(void);
void kprint_isr(const char* str);
void kprint_hex(u8 value);
extern u16 row;
extern u16 col;
extern u16 input_start_row;
extern u16 input_start_col;
void update_cursor(u16 row, u16 col);
void kprint_dec(u32 num);
#endif