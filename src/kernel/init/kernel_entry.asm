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
;   File: kernel_entry.asm
;   Created on: August 10th 2025
;   Created by: jjones (GitHub Username: KlondikeDev)
;   Purpose: To setup the kernel entry point.
;   Dependencies: None (Needs the bootloader, but that should be obvious.)
;    
;   Suggested Changes/Todo:
;   Nothing to do.
;
;

[BITS 32]
[EXTERN kmain]

global _start

section .text
_start:
    ; We're already in protected mode with segments set up
    ; Just need to ensure stack is properly aligned and call kmain
    
    ; Set up stack (ensure 16-byte alignment for modern calling conventions)
    mov esp, 0x90000
    and esp, 0xFFFFFFF0  ; Align to 16 bytes
    
    ; Clear direction flag
    cld
    
    ; Push a fake return address (we should never return from kmain)
    push 0
    
    ; Call the C kernel main function
    call kmain
    
    ; If kmain returns (it shouldn't), halt
.halt:
    cli
    hlt
    jmp .halt