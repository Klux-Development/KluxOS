; Multiboot header
section .multiboot
align 4
    dd 0x1BADB002              ; Multiboot magic number
    dd 0x00000003              ; Flags: page align, memory info
    dd -(0x1BADB002+0x00000003); Checksum


section .text
global _start
_start:
    
    mov esp, stack_top
    
    extern kernel_main
    push ebx                
    call kernel_main
    
    
.loop:
    cli
    hlt
    jmp .loop
    
section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KB
stack_top: