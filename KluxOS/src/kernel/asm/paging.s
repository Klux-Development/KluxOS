# Paging functions

.section .text
.align 4

.global load_page_directory
load_page_directory:
    push %ebp
    mov %esp, %ebp
    mov 8(%ebp), %eax   
    mov %eax, %cr3      
    pop %ebp
    ret

.global enable_paging
enable_paging:
    push %ebp
    mov %esp, %ebp
    mov %cr0, %eax      
    or $0x80000000, %eax 
    mov %eax, %cr0       
    pop %ebp
    ret

.global disable_paging
disable_paging:
    push %ebp
    mov %esp, %ebp
    mov %cr0, %eax      
    and $0x7FFFFFFF, %eax 
    mov %eax, %cr0       
    pop %ebp
    ret

.global get_page_directory
get_page_directory:
    push %ebp
    mov %esp, %ebp
    mov %cr3, %eax      
    pop %ebp
    ret

.global flush_tlb
flush_tlb:
    push %ebp
    mov %esp, %ebp
    mov %cr3, %eax      
    mov %eax, %cr3      
    pop %ebp
    ret

.global flush_tlb_single
flush_tlb_single:
    push %ebp
    mov %esp, %ebp
    mov 8(%ebp), %eax    
    invlpg (%eax)        
    pop %ebp
    ret 