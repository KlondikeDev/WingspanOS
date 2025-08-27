/*
    File: idt.c
    Created on: August 8th 2025
    Created by: jjones (GitHub Username: KlondikeDev)
    Purpose: Interrupt Descriptor Table.
    Dependencies: types.h, idt.h, vga.h, kutils.h

    Suggested Changes/Todo:
    Nothing Yet.

*/

#include "types.h"
#include "idt.h"
#include "vga.h"
#include "kutils.h"

void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags);
void idt_init(void);
void irq_ack(u8 irq);
void irq_handler(registers_t* regs);

// PIC ports
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

// PIC initialization command words
#define ICW1_ICW4       0x01    // ICW4 (not) needed
#define ICW1_SINGLE     0x02    // Single (cascade) mode
#define ICW1_INTERVAL4  0x04    // Call address interval 4 (8)
#define ICW1_LEVEL      0x08    // Level triggered (edge) mode
#define ICW1_INIT       0x10    // Initialization - required!

#define ICW4_8086       0x01    // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO       0x02    // Auto (normal) EOI
#define ICW4_BUF_SLAVE  0x08    // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C    // Buffered mode/master
#define ICW4_SFNM       0x10    // Special fully nested (not)

// Remap the PIC
void irq_remap(void) {
    u8 a1, a2;
    
    // Save masks
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);
    
    // Start initialization sequence (in cascade mode)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    
    // ICW2: Master PIC vector offset (32)
    outb(PIC1_DATA, 32);
    // ICW2: Slave PIC vector offset (40)  
    outb(PIC2_DATA, 40);
    
    // ICW3: Tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC1_DATA, 4);
    // ICW3: Tell Slave PIC its cascade identity (0000 0010)
    outb(PIC2_DATA, 2);
    
    // ICW4: Have the PICs use 8086 mode (and not 8080 mode)
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    
    // Restore saved masks (but we'll set our own)
    outb(PIC1_DATA, 0xFC);  // Mask all except IRQ 0 (timer) and IRQ 1 (keyboard)
    outb(PIC2_DATA, 0xFF);  // Mask all slave PIC interrupts
}

// Install IRQ handlers
void irq_install(void) {
    irq_remap();
    
    // Install IRQ handlers in IDT
    idt_set_gate(32, (u32)irq0, CODE_SEG, IDT_INTERRUPT_GATE);  // Timer
    idt_set_gate(33, (u32)irq1, CODE_SEG, IDT_INTERRUPT_GATE);  // Keyboard
}

// Static IDT table and descriptor
static idt_entry_t idt[IDT_ENTRIES];
static idt_descriptor_t idt_desc;

// Set a gate in the IDT
void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags) {
    idt[num].offset_low = base & 0xFFFF;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
}

// Initialize the IDT
void idt_init(void) {
    idt_desc.limit = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
    idt_desc.base = (u32)&idt;
    
    // Clear the IDT
    for(int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    
    // Install exception handlers
    idt_set_gate(0, (u32)isr0, CODE_SEG, IDT_INTERRUPT_GATE);   // Divide by zero
    idt_set_gate(1, (u32)isr1, CODE_SEG, IDT_INTERRUPT_GATE);   // Debug
    idt_set_gate(2, (u32)isr2, CODE_SEG, IDT_INTERRUPT_GATE);   // NMI
    idt_set_gate(3, (u32)isr3, CODE_SEG, IDT_INTERRUPT_GATE);   // Breakpoint
    idt_set_gate(4, (u32)isr4, CODE_SEG, IDT_INTERRUPT_GATE);   // Overflow
    idt_set_gate(5, (u32)isr5, CODE_SEG, IDT_INTERRUPT_GATE);   // Bound range exceeded
    idt_set_gate(6, (u32)isr6, CODE_SEG, IDT_INTERRUPT_GATE);   // Invalid opcode
    idt_set_gate(7, (u32)isr7, CODE_SEG, IDT_INTERRUPT_GATE);   // Device not available
    idt_set_gate(8, (u32)isr8, CODE_SEG, IDT_INTERRUPT_GATE);   // Double fault
    idt_set_gate(9, (u32)isr9, CODE_SEG, IDT_INTERRUPT_GATE);   // Coprocessor segment overrun
    idt_set_gate(10, (u32)isr10, CODE_SEG, IDT_INTERRUPT_GATE); // Invalid TSS
    idt_set_gate(11, (u32)isr11, CODE_SEG, IDT_INTERRUPT_GATE); // Segment not present
    idt_set_gate(12, (u32)isr12, CODE_SEG, IDT_INTERRUPT_GATE); // Stack fault
    idt_set_gate(13, (u32)isr13, CODE_SEG, IDT_INTERRUPT_GATE); // General protection fault
    idt_set_gate(14, (u32)isr14, CODE_SEG, IDT_INTERRUPT_GATE); // Page fault
    idt_set_gate(15, (u32)isr15, CODE_SEG, IDT_INTERRUPT_GATE); // Reserved
    idt_set_gate(16, (u32)isr16, CODE_SEG, IDT_INTERRUPT_GATE); // Floating point exception
    
    idt_install();
    irq_install();
    __asm__ volatile ("sti");
}

// Load IDT using assembly
void idt_install(void) {
    __asm__ volatile ("lidt %0" : : "m" (idt_desc));
}

// ISR handler in C
void isr_handler(registers_t* regs) {
    switch(regs->int_no) {
        case 0:
            kprint_isr("KERNEL PANIC: DIVIDE BY ZERO. STOP.\n");
            break;
        case 1:
            kprint_isr("KERNEL PANIC: DEBUG EXCEPTION. STOP.\n");
            break;
        case 2:
            kprint_isr("KERNEL PANIC: NON-MASKABLE INTERRUPT. STOP.\n");
            break;
        case 3:
            kprint_isr("KERNEL PANIC: BREAKPOINT EXCEPTION. STOP.\n");
            break;
        case 4:
            kprint_isr("KERNEL PANIC: OVERFLOW EXCEPTION. STOP.\n");
            break;
        case 5:
            kprint_isr("KERNEL PANIC: BOUND RANGE EXCEEDED. STOP.\n");
            break;
        case 6:
            kprint_isr("KERNEL PANIC: INVALID OPCODE. STOP.\n");
            break;
        case 7:
            kprint_isr("KERNEL PANIC: DEVICE NOT AVAILABLE. STOP.\n");
            break;
        case 8:
            kprint_isr("KERNEL PANIC: DOUBLE FAULT. STOP.\n");
            break;
        case 9:
            kprint_isr("KERNEL PANIC: COPROCESSOR SEGMENT OVERRUN. STOP.\n");
            break;
        case 10:
            kprint_isr("KERNEL PANIC: INVALID TSS. STOP.\n");
            break;
        case 11:
            kprint_isr("KERNEL PANIC: SEGMENT NOT PRESENT. STOP.\n");
            break;
        case 12:
            kprint_isr("KERNEL PANIC: STACK FAULT. STOP.\n");
            break;
        case 13:
            kprint_isr("KERNEL PANIC: GENERAL PROTECTION FAULT. STOP.\n");
            break;
        case 14:
            kprint_isr("KERNEL PANIC: PAGE FAULT. STOP.\n");
            break;
        case 16:
            kprint_isr("KERNEL PANIC: FLOATING POINT EXCEPTION. STOP.\n");
            break;
        default:
            kprint_isr("KERNEL PANIC: UNKNOWN ERROR. STOP.\n");
            break;
    }
    
    while(1) { __asm__ volatile ("hlt"); }
}
// Send End of Interrupt to PIC
void irq_ack(u8 irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, 0x20);  // Send EOI to slave PIC
    }
    outb(PIC1_COMMAND, 0x20);      // Send EOI to master PIC
}

void irq_handler(registers_t* regs) {
    switch(regs->int_no) {
        case 32:  // Timer
            break;
        case 33:  // Keyboard
            read_key_from_port();
            break;
        default:
            kprint("Unknown IRQ!\n");
            break;
    }
    
    // Send End of Interrupt signal
    irq_ack(regs->int_no - 32);
}
