/*
    File: kutils.c
    Created on: August 8th 2025
    Created by: jjones (GitHub Username: KlondikeDev)
    Purpose: To setup utility functions for the OS.
    Dependencies: types.h, vga.h, idt.h, kutils.h

    Suggested Changes/Todo:
    Anything, it's just a place to store functions.

*/


#include "idt.h"
#include "vga.h"
#include "types.h"
#include "kutils.h"

extern u8 inb(u16 port);
extern void outb(u16 port, u8 val);

// Simple strcmp equals
bool str_equals(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return false;
        a++; b++;
    }
    return *a == 0 && *b == 0;
}

bool starts_with(const char* str, const char* prefix) {
    while (*prefix) {
        if (*prefix != *str) {
            return false;
        }
        prefix++;
        str++;
    }
    return true;
}

void reboot_system(void) {
    // Wait for keyboard controller input buffer to be empty
    while (inb(0x64) & 0x02);

    // Send the reset command 0xFE to the keyboard controller
    outb(0x64, 0xFE);

    // If that doesn't work, hang here
    while(1) {
        __asm__ volatile ("hlt");
    }
}

void play_sound(u16 frequency, u16 duration) {
    
    u32 div = 1193180 / frequency;
    
    outb(0x43, 0xb6);
    outb(0x42, (u8)(div));
    outb(0x42, (u8)(div >> 8));
    
    u8 tmp = inb(0x61);
    
    outb(0x61, tmp | 3);
    
    tmp = inb(0x61);
    
    // This prevents optimization better
    volatile u32 dummy = 0;
    for(volatile u32 i = 0; i < duration * 100000; i++) {
        dummy++;  // Give the loop something to do
        __asm__ volatile ("" ::: "memory");  // Memory barrier
    }
    nosound();
    kprint("Sound finished\n");
}

void simple_beep() {
    kprint("Simple beep test\n");
    outb(0x61, inb(0x61) | 3);
    for(u32 i = 0; i < 50000000; i++) __asm__ volatile ("nop");
    outb(0x61, inb(0x61) & 0xFC);
    kprint("Simple beep done\n");
}

void nosound() {
 	uint8_t tmp = inb(0x61) & 0xFC;
     
 	outb(0x61, tmp);
}

u32 str_to_uint(const char* str) {
    u32 result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

// RTC Reading Functions
u8 read_cmos(u8 address) {
    outb(0x70, 0x0A);
    while (inb(0x71) & 0x80) { } // Wait until update finishes

    outb(0x70, address);
    return inb(0x71);
}

u8 bcd_to_bin(u8 val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

u8 get_rtc_seconds() {
    u8 seconds = read_cmos(0x00);
    outb(0x70, 0x0B);
    u8 status_b = inb(0x71);
    if (!(status_b & 0x04)) seconds = bcd_to_bin(seconds);
    return seconds;
}

u8 get_rtc_minutes() {
    u8 minutes = read_cmos(0x02);
    outb(0x70, 0x0B);
    u8 status_b = inb(0x71);
    if (!(status_b & 0x04)) minutes = bcd_to_bin(minutes);
    return minutes;
}

u8 get_rtc_hours() {
    u8 hours = read_cmos(0x04);
    outb(0x70, 0x0B);
    u8 status_b = inb(0x71);
    if (!(status_b & 0x04)) hours = bcd_to_bin(hours);
    return hours;
}

u8 get_rtc_day() {
    u8 day = read_cmos(0x07);
    outb(0x70, 0x0B);
    u8 status_b = inb(0x71);
    if (!(status_b & 0x04)) day = bcd_to_bin(day);
    return day;
}

u8 get_rtc_month() {
    u8 month = read_cmos(0x08);
    outb(0x70, 0x0B);
    u8 status_b = inb(0x71);
    if (!(status_b & 0x04)) month = bcd_to_bin(month);
    return month;
}

u8 get_rtc_year() {
    u8 year = read_cmos(0x09);
    outb(0x70, 0x0B);
    u8 status_b = inb(0x71);
    if (!(status_b & 0x04)) year = bcd_to_bin(year);
    return year;
}

void outb(u16 port, u8 val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

u8 inb(u16 port) {
    u8 ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}