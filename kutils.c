#include "idt.h"
#include "vga.h"
#include "types.h"

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
