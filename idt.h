#pragma once
#include "types.h"

// IDT Entry structure (8 bytes)
typedef struct {
    u16 offset_low;    // Lower 16 bits of handler address
    u16 selector;      // Code segment selector (from GDT)
    u8  zero;          // Always 0
    u8  type_attr;     // Gate type and attributes
    u16 offset_high;   // Upper 16 bits of handler address
} PACKED idt_entry_t;

// IDT Descriptor structure (6 bytes)
typedef struct {
    u16 limit;         // Size of IDT - 1
    u32 base;          // Address of IDT
} PACKED idt_descriptor_t;

// Registers structure for interrupt handlers
typedef struct {
    u32 ds;                                     // Data segment selector
    u32 eax, ecx, edx, ebx, esp, ebp, esi, edi; // Correct pusha order
    u32 int_no, err_code;                       // Interrupt number and error code
    u32 eip, cs, eflags, useresp, ss;           // Pushed by processor
} registers_t;
// Constants
#define IDT_ENTRIES 256
#define IDT_INTERRUPT_GATE 0x8E   // Present, Ring 0, 32-bit interrupt gate
#define CODE_SEG 0x08             // Your code segment from GDT

// Function declarations
void idt_init(void);
void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags);
void idt_install(void);
void isr_handler(registers_t* regs);

// Exception handler declarations
void isr0(void);   void isr1(void);   void isr2(void);   void isr3(void);
void isr4(void);   void isr5(void);   void isr6(void);   void isr7(void);
void isr8(void);   void isr9(void);   void isr10(void);  void isr11(void);
void isr12(void);  void isr13(void);  void isr14(void);  void isr15(void);
void isr16(void);

// IRQ handler declarations
void irq0(void);
void irq1(void);
// IRQ handler function
void irq_handler(registers_t* regs);

// PIC functions
void irq_remap(void);
void irq_install(void);

void read_key_from_port(void);