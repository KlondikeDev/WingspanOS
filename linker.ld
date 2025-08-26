ENTRY(_start)

SECTIONS
{
    . = 0x1000;
    
    .text : {
        *(.text.entry)  /* Entry point goes first */
        *(.text)
    }
    
    .rodata : {
        *(.rodata)
    }
    
    .data : {
        *(.data)
    }
    
    .bss : {
        *(COMMON)
        *(.bss)
    }
}