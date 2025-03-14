[bits 32]
global isr0
global isr1
; ... diğer ISR'lar
global irq0
global irq1
; ... diğer IRQ'lar

extern isr_handler
extern irq_handler

; CPU Exception Handlers
isr0:
    cli                     ; disable interrupts
    push byte 0             ; exception code
    push byte 0             ; exception number
    jmp isr_common_stub     ; common handler

isr1:
    cli
    push byte 0
    push byte 1
    jmp isr_common_stub

isr_common_stub:
    pusha                  
    

    mov ax, ds
    push eax
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    

    push esp                ; exception number and error code
    call isr_handler
    add esp, 4
    

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                    
    add esp, 8              
    sti                     
    iret                    

; IRQ Handlers (hardware interrupts)
irq0:
    cli
    push byte 0
    push byte 32            ; IRQ0 + 32
    jmp irq_common_stub

irq1:
    cli
    push byte 0
    push byte 33            ; IRQ1 + 32
    jmp irq_common_stub


irq_common_stub:
    pusha
    

    mov ax, ds
    push eax
    

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call irq_handler
    add esp, 4
    
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa
    add esp, 8
    sti
    iret
