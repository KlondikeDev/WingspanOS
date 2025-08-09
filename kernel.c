#include "types.h"
#include "vga.h"
#include "idt.h"
#include "kutils.h"

u16 input_start_row = 0;
u16 input_start_col = 0;

char input_buffer[256];
u32 input_pos = 0;
bool line_ready = false;
void line_completed();
bool str_equals(const char* str1, const char* str2);

void kmain(){
    kprint("Kernel started!\n");
    
    kprint("Initializing IDT...\n");
    idt_init();
    kprint("\nIDT initialized successfully!\n");
    kprint("Hello from kMain!\n");
    kprint("Copyright (C) Joseph Jones // Klondike Software\n");

    kprint("kunix-$ ");
    update_cursor(row, col);
    input_start_row = row;
    input_start_col = col;
    while(1){
        if(line_ready == true){
            // Copy buffer to command
            static char command[256];
            for(u32 i = 0; i < input_pos; i++) {
                command[i] = input_buffer[i];
            }
            command[input_pos] = '\0';

            kprint("\n");  // <-- Add this here, moves to next line for output
            
            // Process command
            if(str_equals(command, "help")){
                kprint("Available commands: help, wash, about\n");
            } else if(str_equals(command, "wash")){
                klear();
                row = 0;
                col = 0;
            } else if(str_equals(command, "about")){
                kprint("Kunix v0.0.0-rcOne");
            } else if(starts_with(command, "echo ")){
                kprint(command + 5);  // print everything after "echo "
                kprint("\n");
            } else{
                kprint("Syntax Error. 1\n");
            }
            // Reset for next command
            input_pos = 0;
            line_ready = false;
            kprint("kunix-$ ");
            input_start_row = row;
            input_start_col = col;
            update_cursor(row, col);  // <-- ADD THIS
        }
        __asm__ volatile ("hlt");
    }
}

void line_completed() {
    line_ready = true;
}

bool str_equals(const char* str1, const char* str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return false;
        }
        str1++;
        str2++;
    }
    return *str1 == *str2;  // Both should be '\0' if equal
}