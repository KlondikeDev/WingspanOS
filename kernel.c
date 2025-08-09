#include "types.h"
#include "vga.h"
#include "idt.h"
#include "kutils.h"

// Input handling
u16 input_start_row = 0;
u16 input_start_col = 0;

char input_buffer[256];
u32 input_pos = 0;
bool line_ready = false;

void line_completed();
bool str_equals(const char* str1, const char* str2);

void kmain() {
    kprint("Kernel started!\n");

    kprint("Initializing IDT...\n");
    idt_init();
    kprint("\nIDT initialized successfully!\n");
    kprint("Hello from kMain!\n");
    kprint("Copyright (C) Joseph Jones // Klondike Software\n");

    kprint("kunix-$ ");
    update_cursor(row, col);
    input_start_row = row;
    input_start_col = col;

    while (1) {
        if (line_ready == true) {
            static char command[256];
            for (u32 i = 0; i < input_pos; i++) {
                command[i] = input_buffer[i];
            }
            command[input_pos] = '\0';

            kprint("\n");

            // Commands
            if (str_equals(command, "help")) {
                kprint("Available commands: help, wash, about, reboot, echo, play, rtc\n");
            }
            else if (str_equals(command, "wash")) {
                klear();
                row = 0;
                col = 0;
            }
            else if (str_equals(command, "about")) {
                kprint("Kunix v0.0.0-rcOne\n");
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
            else {
                kprint("Syntax Error. 1\n");
            }

            input_pos = 0;
            line_ready = false;
            kprint("kunix-$ ");
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