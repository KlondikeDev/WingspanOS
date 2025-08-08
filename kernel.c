#include "types.h"
#include "vga.h"
#include "idt.h"

void kmain(){
    kprint("Kernel started!\n");
    
    kprint("Initializing IDT...\n");
    idt_init();
    kprint("\nIDT initialized successfully!\n");
    kprint("Testing if interrupts work...\n");
    int x = 5, y = 0;
    int result = x / y;  // Should still trigger divide by zero
    kprint("Hello from kMain!\n");
    kprint("Copyright (C) Joseph Jones // Klondike Software\n");


    while(1){
        __asm__ volatile ("hlt");
    }
}