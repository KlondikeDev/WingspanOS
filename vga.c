#include "types.h"
#include "vga.h"

void kprint(const char* str){
    u16* VGA_MEMORY = (u16*) (0xB8000);
    static u16 cursorPos = 3;  // Start after AB from bootloader
    
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