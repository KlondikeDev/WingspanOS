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