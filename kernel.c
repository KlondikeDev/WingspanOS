#include "types.h"
#include "vga.h"

void kmain(){
    kprint("Hello from kMain!\n");
    while(1){
        __asm__ volatile ("hlt");
    }
}