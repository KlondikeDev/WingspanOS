[BITS 32]
[ORG 0x1000]

start:
    ; Clear screen with green background
    mov edi, 0xB8000
    mov ecx, 80 * 25
    mov ax, 0x2F20  ; Green background, white foreground, space
    rep stosw
    
    ; Print "KERNEL OK!" at top of screen
    mov edi, 0xB8000
    mov esi, kernel_msg
    mov ah, 0x2F  ; Green background, white text
    
.print_loop:
    lodsb
    test al, al
    jz .done
    stosb
    mov al, ah
    stosb
    jmp .print_loop
    
.done:
    ; Infinite loop
    cli
.halt:
    hlt
    jmp .halt

kernel_msg db 'KERNEL OK! - If you see this, the boot process works!', 0

; Pad to make sure we have enough size
times 512-($-$$) db 0