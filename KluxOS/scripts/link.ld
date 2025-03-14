/* KluxOS Bağlayıcı Betiği */
OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS
{
    . = 0x00100000;
    
    .text : {
        *(.multiboot) 
        *(.text)
        . = ALIGN(4K);
    }
    
    .rodata : {
        *(.rodata)
        . = ALIGN(4K);
    }
    
    .data : {
        *(.data)
        . = ALIGN(4K);
    }
    
    .bss : {
        *(COMMON)
        *(.bss)
        . = ALIGN(4K);
    }
    
    /* Çekirdeğin sonunu işaretleme */
    _kernel_end = .;
}
