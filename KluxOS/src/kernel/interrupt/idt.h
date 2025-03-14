#ifndef IDT_H
#define IDT_H

#include <kernel/types.h>

// IDT entry flags
#define IDT_FLAG_PRESENT     0x80   // interrupt handler exists
#define IDT_FLAG_RING0       0x00   // kernel mode (ring 0)
#define IDT_FLAG_RING1       0x20   // ring 1
#define IDT_FLAG_RING2       0x40   // ring 2
#define IDT_FLAG_RING3       0x60   // user mode (ring 3)
#define IDT_FLAG_32BIT       0x0E   // 32-bit interrupt gate
#define IDT_FLAG_TRAP        0x0F   // trap gate

typedef struct {
    uint16_t base_low;      // interrupt handler address low 16 bits
    uint16_t selector;      // kernel code segment selector
    uint8_t  reserved;      // always 0
    uint8_t  flags;         // flags
    uint16_t base_high;     // interrupt handler address high 16 bits
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
idt_entry_t;

typedef struct {
    uint16_t limit;         // IDT size - 1
    uint32_t base;          // IDT base address
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
idt_ptr_t;

typedef void (*isr_t)(uint32_t error_code);

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);
void idt_load(idt_ptr_t* ptr);
void register_interrupt_handler(uint8_t num, isr_t handler);


extern void isr0(void);   // division by zero error
extern void isr1(void);   // debug exception
extern void isr2(void);   // non-maskable interrupt
extern void isr3(void);   // breakpoint exception
extern void isr4(void);   // overflow exception
extern void isr5(void);   // bound range exceeded
extern void isr6(void);   // invalid opcode
extern void isr7(void);   // device not available
extern void isr8(void);   // double fault
extern void isr9(void);   // coprocessor segment overrun
extern void isr10(void);  // invalid TSS
extern void isr11(void);  // segment not present
extern void isr12(void);  // stack segment fault
extern void isr13(void);  // general protection fault
extern void isr14(void);  // page fault
extern void isr15(void);  // reserved
extern void isr16(void);  // x87 FPU error
extern void isr17(void);  // alignment check error
extern void isr18(void);  // machine check error
extern void isr19(void);  // SIMD FPU error
extern void isr20(void);  // virtualization error
extern void isr21(void);  // reserved
extern void isr22(void);  // reserved
extern void isr23(void);  // reserved
extern void isr24(void);  // reserved
extern void isr25(void);  // reserved
extern void isr26(void);  // reserved
extern void isr27(void);  // reserved
extern void isr28(void);  // reserved
extern void isr29(void);  // reserved
extern void isr30(void);  // security exception
extern void isr31(void);  // reserved

// IRQ handlers
extern void irq0(void);   // timer
extern void irq1(void);   // keyboard
extern void irq2(void);   // PIC cascade
extern void irq3(void);   // COM2
extern void irq4(void);   // COM1
extern void irq5(void);   // LPT2
extern void irq6(void);   // floppy
extern void irq7(void);   // LPT1
extern void irq8(void);   // CMOS RTC
extern void irq9(void);   // empty / ACPI
extern void irq10(void);  // empty / SCSI / NIC
extern void irq11(void);  // empty / SCSI / NIC
extern void irq12(void);  // PS/2 Mouse
extern void irq13(void);  // FPU / Coprocessor
extern void irq14(void);  // ATA primary disk
extern void irq15(void);  // ATA secondary disk

#define IRQ0  32
#define IRQ1  33
#define IRQ2  34
#define IRQ3  35
#define IRQ4  36
#define IRQ5  37
#define IRQ6  38
#define IRQ7  39
#define IRQ8  40
#define IRQ9  41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

#endif // IDT_H 