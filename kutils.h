/*
    File: kutils.h
    Created on: August 8th 2025
    Created by: jjones (GitHub Username: KlondikeDev)
    Purpose: The header file for kutils.
    Dependencies: types.h, idt.h

    Suggested Changes/Todo:
    Anything, it's just a place to store functions.

*/

#pragma once

#include "types.h"
#include "idt.h"

bool starts_with(const char* str, const char* prefix);
bool str_equals(const char* a, const char* b);
void reboot_system(void);
u32 str_to_uint(const char* str);
void play_sound(u16 frequency, u16 duration);
void nosound();
void simple_beep();
void line_completed();
void save_to_history(const char* command);
bool str_equals(const char* str1, const char* str2);
u8 read_cmos(u8 address);
u8 get_rtc_seconds();
u8 get_rtc_minutes();
u8 get_rtc_hours();
u8 get_rtc_day(); 
u8 get_rtc_year();     
u8 get_rtc_month();
void outb(u16 port, u8 val);
u8 inb(u16 port);
static inline u16 inw(u16 port) {
    u16 ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(u16 port, u16 val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}