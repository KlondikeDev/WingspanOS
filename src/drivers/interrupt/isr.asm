;
; Copyright 2025 Joseph Jones
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;     http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.
;
;   File: isr.asm
;   Created on: August 8th 2025
;   Created by: jjones (GitHub Username: KlondikeDev)
;   Purpose: Set up interrupt service routines (ISRs) for CPU exceptions and hardware interrupts.
;   Dependencies: None (None apparent, anyway. Still needs the IDT and GDT. Duh.)
;    
;   Suggested Changes/Todo:
;   Nothing to do.
;
;

[BITS 32]

;=============================================================================
; CPU Exception Handlers (ISRs)
;=============================================================================

; Macro for ISRs that don't push error codes
; Modify the ISR_NOERRCODE macro to add debugging:
%macro ISR_NOERRCODE 1
    global isr%1
    isr%1:
        ; Debug: Write to VGA to show we reached the ISR
        
        cli
        push 0
        push %1
        jmp isr_common_stub
%endmacro

; Macro for ISRs that do push error codes  
%macro ISR_ERRCODE 1
    global isr%1
    isr%1:
        cli
        push %1                ; Push interrupt number (error code already pushed)
        jmp isr_common_stub
%endmacro

; Create ISR stubs for CPU exceptions (0-31)
ISR_NOERRCODE 0     ; Divide by zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non-maskable interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Overflow
ISR_NOERRCODE 5     ; Bound range exceeded
ISR_NOERRCODE 6     ; Invalid opcode
ISR_NOERRCODE 7     ; Device not available
ISR_ERRCODE   8     ; Double fault
ISR_NOERRCODE 9     ; Coprocessor segment overrun
ISR_ERRCODE   10    ; Invalid TSS
ISR_ERRCODE   11    ; Segment not present
ISR_ERRCODE   12    ; Stack fault
ISR_ERRCODE   13    ; General protection fault
ISR_ERRCODE   14    ; Page fault
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; Floating point exception

; Common ISR handler for CPU exceptions
extern isr_handler
isr_common_stub:
    pusha                      ; Push all general purpose registers
    
    mov ax, ds                 ; Save data segment
    push eax
    
    mov ax, 0x10               ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov eax, esp               ; Pass pointer to register structure
    push eax
    call isr_handler           ; Call C handler
    add esp, 4                 ; Clean up parameter
    
    pop eax                    ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                       ; Restore all registers
    add esp, 8                 ; Clean up error code and interrupt number
    sti                        ; Re-enable interrupts
    iret                       ; Return from interrupt

;=============================================================================
; Hardware Interrupt Handlers (IRQs)  
;=============================================================================

; Macro for IRQ handlers
%macro IRQ 2
    global irq%1
    irq%1:
        cli
        push 0                 ; Dummy error code
        push %2                ; IRQ number (32 + IRQ)
        jmp irq_common_stub
%endmacro

; Create IRQ handlers
IRQ 0, 32                      ; Timer (IRQ 0 → Interrupt 32)
IRQ 1, 33                      ; Keyboard (IRQ 1 → Interrupt 33)

; Common IRQ handler for hardware interrupts
extern irq_handler
irq_common_stub:
    pusha                      ; Push all general purpose registers
    
    mov ax, ds                 ; Save data segment
    push eax
    
    mov ax, 0x10               ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov eax, esp               ; Pass pointer to register structure
    push eax
    call irq_handler           ; Call C handler
    add esp, 4                 ; Clean up parameter
    
    pop eax                    ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                       ; Restore all registers
    add esp, 8                 ; Clean up error code and IRQ number
    sti                        ; Re-enable interrupts
    iret                       ; Return from interrupt