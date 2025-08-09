#include "types.h"
#include "vga.h"
#include "idt.h"

u16 input_start_row = 0;
u16 input_start_col = 0;

void kmain(){
    kprint("Kernel started!\n");
    
    kprint("Initializing IDT...\n");
    idt_init();
    kprint("\nIDT initialized successfully!\n");
    kprint("Hello from kMain!\n");
    kprint("Copyright (C) Joseph Jones // Klondike Software\n");

    kprint("kunix-$ ");
    input_start_row = row;
    input_start_col = col;

    while(1){
        __asm__ volatile ("hlt");
    }
}