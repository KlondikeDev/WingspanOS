;
;   File: stage2.asm
;   Created on: August 7th 2025
;   Created by: jjones (GitHub Username: KlondikeDev)
;   Purpose: To boot the OS. (Bootloader stage 2)
;   Dependencies: None (Second Stage Bootloader, so it needs the first stage. But if you didn't know that, maybe read about OSes first?)
;
;   Suggested Changes/Todo:
;   Nothing to do, unless the kernel gets too big.
;
;


[ORG 0x8000]
[BITS 16]

stage2_start:
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Set up stack for Stage 2
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Print boot drive immediately so we can see DL (0x00 floppy, 0x80 HDD)
    mov si, boot_drive_msg
    call print_string
    mov al, [boot_drive]
    call print_hex_byte
    mov si, newline
    call print_string

    ; Jump past utility function definitions to runtime start
    jmp after_funcs

load_kernel:
    mov si, kernel_load_msg
    call print_string

    ; Reset disk system
    xor ax, ax
    mov dl, [boot_drive]
    int 0x13
    jc kernel_error

    ; We'll load the kernel in smaller chunks to avoid BIOS limitations
    ; Start loading at 0x1000
    mov ax, 0x0100
    mov es, ax
    xor bx, bx              ; ES:BX = 0x0100:0x0000 = 0x1000

    ; Kernel starts at sector 10 (after stage2) 
    mov byte [current_sector], 10
    mov byte [current_head], 0
    mov byte [current_cylinder], 0
    mov word [sectors_loaded], 0

    ; DEBUG: Print INITIAL values before any loading
    mov si, initial_chs_msg
    call print_string
    mov al, [current_cylinder]
    call print_hex_byte
    mov al, '/'
    mov ah, 0x0E
    int 0x10
    mov al, [current_head] 
    call print_hex_byte
    mov al, '/'
    mov ah, 0x0E
    int 0x10
    mov al, [current_sector]
    call print_hex_byte
    mov si, newline
    call print_string

.load_loop:
    ; Check if we've loaded enough (48 sectors - kernel needs more than 36)
    cmp word [sectors_loaded], 48
    jae .done_loading

    ; Read one sector at a time (safe CHS handling)
    mov byte [sectors_to_read], 1

    ; Debug: print CHS and count
    mov si, chs_msg
    call print_string
    mov al, [current_cylinder]
    call print_hex_byte
    mov al, '/'
    mov ah, 0x0E
    int 0x10
    mov al, [current_head]
    call print_hex_byte
    mov al, '/'
    mov ah, 0x0E
    int 0x10
    mov al, [current_sector]
    call print_hex_byte
    mov si, count_msg
    call print_string
    movzx ax, byte [sectors_to_read]
    call print_dec
    mov si, newline
    call print_string

    ; Read sectors
    mov ah, 0x02            ; Read function
    mov al, [sectors_to_read]
    mov ch, [current_cylinder]
    mov cl, [current_sector]
    mov dh, [current_head]
    mov dl, [boot_drive]
    int 0x13
    jc kernel_error

    ; Update loaded count
    movzx ax, byte [sectors_to_read]
    add [sectors_loaded], ax

    ; Update buffer pointer (sectors_to_read * 512 bytes)
    movzx ax, byte [sectors_to_read]
    mov cx, 512
    mul cx                  ; DX:AX = sectors * 512
    add bx, ax
    jnc .no_segment_wrap
    ; Handle segment wrap (BX overflow)
    mov ax, es
    add ax, 0x1000          ; Advance by 64KB
    mov es, ax
    xor bx, bx
.no_segment_wrap:
    ; Increment sector by 1 and handle wrap
    inc byte [current_sector]
    cmp byte [current_sector], 19
    jb .load_loop
    mov byte [current_sector], 1
    inc byte [current_head]
    cmp byte [current_head], 2
    jb .load_loop
    mov byte [current_head], 0
    inc byte [current_cylinder]
    jmp .load_loop

.done_loading:
    mov si, kernel_ok_msg
    call print_string
    mov si, sectors_loaded_msg
    call print_string
    mov ax, [sectors_loaded]
    call print_dec
    mov si, newline
    call print_string
    ; Indicate load complete
    mov si, load_complete_msg
    call print_string
    mov si, newline
    call print_string
    ret

; Utility functions (moved above usage)
print_dec:
    ; Simple decimal print for AX
    push ax
    push bx
    push cx
    push dx
    mov bx, 10
    xor cx, cx
push_digits:
    xor dx, dx
    div bx
    push dx
    inc cx
    test ax, ax
    jnz push_digits
pop_digits:
    pop ax
    add al, '0'
    mov ah, 0x0E
    int 0x10
    loop pop_digits
    pop dx
    pop cx
    pop bx
    pop ax
    ret

print_hex_byte:
    push ax
    shr al, 4
    call print_hex_digit
    pop ax
    and al, 0x0F
    call print_hex_digit
    ret

print_hex_digit:
    add al, '0'
    cmp al, '9'
    jle print_digit
    add al, 7
print_digit:
    mov ah, 0x0E
    int 0x10
    ret

after_funcs:
    ; Print Stage 2 loaded message
    mov si, stage2_msg
    call print_string

    ; Print saved boot drive (DL)
    mov si, boot_drive_msg
    call print_string
    mov al, [boot_drive]
    call print_hex_byte
    mov si, newline
    call print_string

    ; Continue to load the kernel
    call load_kernel
    
    ; Enter protected mode and jump to kernel
    call enter_protected_mode

enter_protected_mode:
    mov si, pmode_msg
    call print_string
    
    cli                     ; Disable interrupts
    lgdt [gdt_descriptor]   ; Load GDT
    
    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Far jump to flush pipeline and enter protected mode
    jmp CODE_SEG:protected_mode_start

[BITS 32]
protected_mode_start:
    ; Set up segments for protected mode
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Set up stack
    mov esp, 0x90000
    
    ; Jump to kernel
        ; Far jump to kernel entry (set CS)
        jmp 0x08:0x1000

[BITS 16]
; Utility functions
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

; Data
boot_drive db 0
current_sector db 0
current_head db 0
current_cylinder db 0
sectors_to_read db 0
sectors_loaded dw 0

; Messages
stage2_msg db 'Stage 2 bootloader loaded', 13, 10, 0
kernel_load_msg db 'Loading kernel...', 13, 10, 0
kernel_ok_msg db 'Kernel loaded successfully!', 13, 10, 0
kernel_error_msg db 'Kernel load failed!', 13, 10, 0
pmode_msg db 'Entering protected mode...', 13, 10, 0
error_code_msg db ' Error: ', 0
failed_at_msg db ' at CHS: ', 0
sectors_loaded_msg db 'Sectors loaded: ', 0
chs_msg db 'CHS: ', 0
count_msg db ' Count: ', 0
newline db 13, 10, 0
boot_drive_msg db 'Boot drive: ', 0
load_complete_msg db 'Load complete, entering protected mode...', 13,10,0
initial_chs_msg db 'INITIAL CHS: ', 0

; GDT setup

kernel_error:
    mov si, kernel_error_msg
    call print_string
    ; Print error code (AH)
    mov al, ah
    call print_hex_byte
    ; Print where it failed
    mov si, failed_at_msg
    call print_string
    mov al, [current_cylinder]
    call print_hex_byte
    mov al, '/'
    mov ah, 0x0E
    int 0x10
    mov al, [current_head]
    call print_hex_byte
    mov al, '/'
    mov ah, 0x0E
    int 0x10
    mov al, [current_sector]
    call print_hex_byte
    jmp $

align 8
gdt_start:
    dd 0x0, 0x0             ; Null descriptor

gdt_code:
    dw 0xFFFF               ; Limit low
    dw 0x0000               ; Base low
    db 0x00                 ; Base middle
    db 10011010b            ; Access byte
    db 11001111b            ; Flags + limit high
    db 0x00                 ; Base high

gdt_data:
    dw 0xFFFF               ; Limit low
    dw 0x0000               ; Base low
    db 0x00                 ; Base middle
    db 10010010b            ; Access byte
    db 11001111b            ; Flags + limit high
    db 0x00                 ; Base high

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size
    dd gdt_start                ; Address

; Segment selectors
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start