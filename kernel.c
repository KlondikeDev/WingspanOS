#include "types.h"
#include "vga.h"
#include "idt.h"

void kmain(){
    kprint("Kernel started!\n");
    
    kprint("Initializing IDT...\n");
    idt_init();
    kprint("\nIDT initialized successfully!\n");
    kprint("Hello from kMain!\n");
    kprint("Copyright (C) Joseph Jones // Klondike Software\n");


    while(1){
        __asm__ volatile ("hlt");
    }
}