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

u16* VGA_MEMORY = (u16*) (0xB8000);
u16 cursorPos = 0;
void kprint(const char* str){  
    for(u32 i = 0; str[i] != '\0'; i++){
        if(str[i] == '\n'){
            cursorPos = (cursorPos / 80 + 1) * 80;
        } else{
            VGA_MEMORY[cursorPos] = (str[i]) | (VGA_COLOR(VGA_BLACK, VGA_WHITE) << 8);
            cursorPos++;
        }

        if(cursorPos >= 80 * 25)
            cursorPos = 0;
    }
}

void klear(){
    for(u32 i = 0; i < 80 * 25; i++){
        VGA_MEMORY[i] = (VGA_COLOR(VGA_BLACK, VGA_WHITE) << 8) | ' ';
    }
    cursorPos = 0;
} 