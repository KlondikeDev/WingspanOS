;
;   File: boot.asm
;   Created on: August 7th 2025
;   Created by: jjones (GitHub Username: KlondikeDev)
;   Purpose: To boot the OS.
;   Dependencies: None (First Stage Bootloader)
;    
;   Suggested Changes/Todo:
;   Nothing to do.
;
;

[ORG 0x7C00]
[BITS 16]

_start:
    xor ax, ax
    mov es, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00

    mov ax, 0
    mov es, ax
    mov bx, 0x8000

    ; Load Stage 2 from disk
    mov ah, 2        ; BIOS read sectors function
    mov al, 8        ; Number of sectors to read (FIXED: was 10, now matches STAGE2_SECTORS)
    mov ch, 0        ; Cylinder 0
    mov cl, 2        ; Start from sector 2 (sector 1 is this bootloader)
    mov dh, 0        ; Head 0
    int 0x13         ; Call BIOS
    jc disk_error    ; Jump if carry flag set (error)
    
    ; Jump to Stage 2
    jmp 0x0000:0x8000

disk_error:
    mov si, error_msg
    call print_string
    jmp $

print_string:
    mov ah, 0x0E
.next_char:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .next_char
.done:
    ret

error_msg db 'Stage 2 load failed', 0

TIMES 510-($-$$) db 0
dw 0xAA55