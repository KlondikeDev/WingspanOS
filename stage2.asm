[ORG 0x8000]
[BITS 16]

stage2_start:
    ; Set up stack for Stage 2
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Print Stage 2 loaded message
    mov si, stage2_msg
    call print_string
    
    ; Detect memory
    call detect_memory
    
    ; Load kernel from disk
    call load_kernel
    
    ; Set up GDT and enter protected mode
    call enter_protected_mode

detect_memory:
    mov si, mem_detect_msg
    call print_string
    
    ; Use BIOS int 0x15, eax=0xE820 to detect memory
    mov di, 0x9000          ; Store memory map at 0x9000
    xor ebx, ebx            ; Start with 0
    mov edx, 0x534D4150     ; 'SMAP' signature
    
.memory_loop:
    mov eax, 0xE820
    mov ecx, 24             ; Size of structure
    int 0x15
    jc .memory_done         ; Carry set = end of list
    
    cmp eax, 0x534D4150     ; Check signature
    jne .memory_error
    
    ; Move to next entry
    add di, 24
    test ebx, ebx           ; If ebx = 0, we're done
    jz .memory_done
    jmp .memory_loop
    
.memory_done:
    mov si, mem_ok_msg
    call print_string
    ret
    
.memory_error:
    mov si, mem_error_msg
    call print_string
    ret

load_kernel:
    mov si, kernel_load_msg
    call print_string
    
    ; Set up for kernel loading
    mov ax, 0
    mov es, ax
    mov bx, 0x1000          ; Load kernel to 0x1000
    
    mov ah, 2               ; Read sectors
    mov al, 10              ; Number of sectors
    mov ch, 0               ; Cylinder 0
    mov cl, 10              ; Start from sector 10
    mov dh, 0               ; Head 0
    
    int 0x13
    jc .kernel_error
    
    mov si, kernel_ok_msg
    call print_string
    ret
    
.kernel_error:
    mov si, kernel_error_msg
    call print_string
    
    ; Print the actual error code
    mov si, error_code_msg
    call print_string
    mov al, ah              ; AH contains error code
    call print_hex_byte
    
    jmp $

error_code_msg db 'Error code: 0x', 0

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
    jle .print
    add al, 7
.print:
    mov ah, 0x0E
    int 0x10
    ret
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
    jmp 0x1000

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

; Messages
stage2_msg db 'Stage 2 bootloader loaded', 13, 10, 0
mem_detect_msg db 'Detecting memory...', 13, 10, 0
mem_ok_msg db 'Memory detection complete', 13, 10, 0
mem_error_msg db 'Memory detection failed', 13, 10, 0
kernel_load_msg db 'Loading kernel...', 13, 10, 0
kernel_ok_msg db 'Kernel loaded successfully', 13, 10, 0
kernel_error_msg db 'Kernel load failed!', 13, 10, 0
pmode_msg db 'Entering protected mode...', 13, 10, 0

; GDT setup
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