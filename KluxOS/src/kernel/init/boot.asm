[bits 32]
section .multiboot
align 8
mb2_header_start:
    dd 0xE85250D6                ; Multiboot2 magic offset
    dd 0                         ; Architecture: 0 (32-bit i386)
    dd mb2_header_end - mb2_header_start ; Header size
    dd -(0xE85250D6 + 0 + (mb2_header_end - mb2_header_start)) ; Checksum
    
    dw 1                         
    dw 0                       
    dd 8                        
    dd 3                        
    
   
    dw 2                         ; Type: Address
    dw 0                         ; Flags
    dd 24                        ; Size
    dd 0                         ; Start address
    dd 0                         ; Load address
    dd 0                         ; End address
    dd 0                         ; BSS end address
    

    dw 0                         ; Type
    dw 0                         ; Flags
    dd 8                         ; Size
mb2_header_end:

; Multiboot1 header
align 4
    dd 0x1BADB002              ; Magic number
    dd 0x00000003              ; Flags
    dd -(0x1BADB002 + 0x03)   ; Checksum

section .bss
align 16
stack_bottom:
    resb 32768                 ; 32 KB stack
stack_top:

section .text
global _start
extern kernel_main

_start:
    cli                       
    mov esp, stack_top         
    cld                   

    push ebx                   
    
  
    call check_a20
    test eax, eax
    jnz a20_enabled
    
 
    call enable_a20
    
a20_enabled:
    call display_boot_info
    
    pop ebx                    
    push ebx                   
    call kernel_main
    
    cli
.hang:
    hlt
    jmp .hang

check_a20:
    pushad
    mov eax, 0x112345          
    mov edi, 0x012345          
    mov esi, 0x112345          
    
    mov [edi], eax             
    mov ebx, [esi]             
    
    cmp eax, ebx               
    popad
    jne .a20_on
    xor eax, eax               
    ret
.a20_on:
    mov eax, 1                 
    ret

enable_a20:
    mov ax, 0x2401
    int 0x15
    ret

display_boot_info:
    push ebp
    mov ebp, esp
    
    mov esp, ebp
    pop ebp
    ret