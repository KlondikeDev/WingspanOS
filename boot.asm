[ORG 0x7C00]
[BITS 16]

_start:
    xor ax, ax
    mov es, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00


    mov ax, 0      ; Set ES first
    mov es, ax
    mov bx, 0x1000

    mov ah, 2
    mov al, 36
    mov ch, 0
    mov cl, 2
    mov dh, 0
    int 0x13
    jc disk_error    ; Jump if carry flag set (error)
    
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp CODE_SEG:protected_mode_start


disk_error:
    mov si, disk_error_msg
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

[BITS 32]
protected_mode_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, 0x90000   
    
    jmp 0x1000
    
    jmp 0x1000

gdt_start:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start                  

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

disk_error_msg db 'ERROR: FAILED TO READ DISK.', 0

TIMES 510-($-$$) db 0
dw 0xAA55