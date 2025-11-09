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
    File: kernel.c
    Created on: August 7th 2025
    Created by: jjones (GitHub Username: KlondikeDev)
    Purpose: To setup the kernel of WingspanOS.
    Dependencies: types.h, vga.h, idt.h, kutils.h, music.h, ata.h

    Suggested Changes/Todo:
    Anything! The kernel in this case, IS THE OS.

*/


#include "types.h"
#include "vga.h"
#include "idt.h"
#include "kutils.h"
#include "music.h"
#include "ata.h"

// Input handling
u16 input_start_row = 0;
u16 input_start_col = 0;

char input_buffer[256];
u32 input_pos = 0;
bool line_ready = false;

void line_completed();
bool str_equals(const char* str1, const char* str2);

void kmain() {
    // CRITICAL: Initialize VGA variables FIRST before any printing
    row = 0;
    col = 0;
    
    // Clear the screen first
    klear();
    
    // Now safe to print
    kprint("Kernel started!\n");

    // Initialize IDT after basic output is working
    idt_init();

    // Initialize ATA/disk
    detect_drives();

    kprint("Copyright (C) 2025 Joseph Jones (KlondikeDev)\n");
    kprint("Licensed under GPL v3\n\n");
    kprint("===========================================\n");
    kprint("     W I N G S P A N   O S   v1.0         \n");
    kprint("===========================================\n");
    kprint("wingspan-$ ");
    update_cursor(row, col);
    input_start_row = row;
    input_start_col = col;

    while (1) {
        if (line_ready) {
            static char command[256];
            for (u32 i = 0; i < input_pos; i++) {
                command[i] = input_buffer[i];
            }
            command[input_pos] = '\0';

            kprint("\n");

            // Commands
            if (str_equals(command, "help")) {
                kprint("Available commands:\n");
                kprint("| UTILITIES:\n");
                kprint("| GENERAL:    help, wash, about, reboot, rtc\n");
                kprint("| FILESYSTEM: drives, format, diskinfo, verify, ls, touch, cat, write, \n");
                kprint("|             rm, cp, find\n");
                kprint("|_\n");
                kprint("MISC: echo, play\n");
            }
            else if (str_equals(command, "wash")) {
                klear();
                row = 0;
                col = 0;
            }
            else if (str_equals(command, "about")) {
                kprint("WingspanOS\n");
            }
            else if (starts_with(command, "echo ")) {
                kprint(command + 5);
                kprint("\n");
            }
            else if (str_equals(command, "reboot")) {
                reboot_system();
            }
            else if (starts_with(command, "play ")) {
                const char* song_name = command + 5;
                play_song(song_name);
            }
            else if (str_equals(command, "play")) {
                kprint("Usage: play <song_name>\n");
                kprint("Available songs: twinkle, mary, frere\n");
            }
            else if (starts_with(command, "rtc ")) {
                const char* rtc_cmd = command + 4;

                if (str_equals(rtc_cmd, "seconds")) {
                    u8 sec = get_rtc_seconds();
                    kprint("Seconds: ");
                    kprint_dec(sec);
                    kprint("\n");
                }
                else if (str_equals(rtc_cmd, "time")) {
                    u8 h = get_rtc_hours();
                    u8 m = get_rtc_minutes();
                    u8 s = get_rtc_seconds();

                    kprint("Time: ");
                    kprint_dec(h); kprint(":");
                    kprint_dec(m); kprint(":");
                    kprint_dec(s);
                    kprint("\n");
                }
                else if (str_equals(rtc_cmd, "date")) {
                    u8 d = get_rtc_day();
                    u8 mo = get_rtc_month();
                    u8 y = get_rtc_year();

                    kprint("Date: ");
                    kprint_dec(d); kprint("/");
                    kprint_dec(mo); kprint("/");
                    kprint_dec(y);
                    kprint("\n");
                }
                else {
                    kprint("Usage: rtc [seconds|time|date]\n");
                }
            }
            else if (str_equals(command, "rtc")) {
                kprint("Usage: rtc [seconds|time|date]\n");
            }
            else if (str_equals(command, "drives")) {
                detect_drives();
            }
            else if (str_equals(command, "format")) {
                kprint("Formatting drive with KLFS...\n");
                klfs_format();
            }
            else if (str_equals(command, "diskinfo")) {
                kprint("Identifying primary master drive...\n");
                identify_drive(0xA0);
            } else if (str_equals(command, "verify")) {
                klfs_verify();
            } else if (str_equals(command, "ls")) {
                klfs_list_files();
            } else if (starts_with(command, "touch ")) {
                const char* filename = command + 6;
                klfs_create_file(filename);
            } else if (str_equals(command, "touch")) {
                kprint("Usage: touch <filename>\n");
            }
            else if (starts_with(command, "write ")) {
                // Format: write filename text
                const char* args = command + 6;
    
                // Find first space (separates filename from text)
                u32 space_pos = 0;
                while(args[space_pos] && args[space_pos] != ' ') space_pos++;
    
                if(args[space_pos] == 0) {
                    kprint("Usage: write <filename> <text>\n");
                } else {
                    // Extract filename
                    char filename[32] = {0};
                    for(u32 i = 0; i < space_pos && i < 31; i++) {
                        filename[i] = args[i];
                    }
        
                    // Text starts after the space
                    const char* text = args + space_pos + 1;
                    klfs_write_file(filename, text);
                }
            } else if (starts_with(command, "cat ")) {
                const char* filename = command + 4;
                klfs_read_file(filename);
            }
            else if (str_equals(command, "cat")) {
                kprint("Usage: cat <filename>\n");
            }
            else if (starts_with(command, "rm ")) {
                const char* filename = command + 3;
                klfs_delete_file(filename);
            }
            else if (str_equals(command, "rm")) {
                kprint("Usage: rm <filename>\n");
            } else if (starts_with(command, "cp ")) {
                const char* args = command + 3;
                u32 space_pos = 0;
                while(args[space_pos] && args[space_pos] != ' ') space_pos++;
    
                if(args[space_pos] == 0) {
                    kprint("Usage: cp <source> <dest>\n");
                } else {
                    char source[32] = {0};
                    for(u32 i = 0; i < space_pos && i < 31; i++) {
                        source[i] = args[i];
                    }
                    const char* dest = args + space_pos + 1;
                    klfs_copy_file(source, dest);
                }
            } else if (starts_with(command, "find ")) {
                const char* pattern = command + 5;
                klfs_find_file(pattern);
            } else {
                kprint("Syntax Error. 1\n");
            }

            input_pos = 0;
            line_ready = false;
            kprint("wingspan-$ ");
            input_start_row = row;
            input_start_col = col;
            update_cursor(row, col);
        }

        __asm__ volatile ("hlt");
    }
}

void line_completed() {
    line_ready = true;
}