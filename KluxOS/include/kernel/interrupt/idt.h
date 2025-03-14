#ifndef IDT_H
#define IDT_H

#include <kernel/types.h>
#include <compat.h>  // Derleyici uyumluluğu için eklendi

#define IDT_ENTRIES 256

// IRQ sabit tanımları 
#define IRQ0  32  // Zamanlayıcı (PIT)
#define IRQ1  33  // Klavye
#define IRQ2  34  // PIC Kaskad
#define IRQ3  35  // COM2
#define IRQ4  36  // COM1
#define IRQ5  37  // LPT2
#define IRQ6  38  // Disket sürücü
#define IRQ7  39  // LPT1
#define IRQ8  40  // CMOS/RTC
#define IRQ9  41  // Boş (ACPI)
#define IRQ10 42  // Boş (SCSI/NIC)
#define IRQ11 43  // Boş (SCSI/NIC)
#define IRQ12 44  // PS/2 Fare
#define IRQ13 45  // FPU
#define IRQ14 46  // ATA Birincil
#define IRQ15 47  // ATA İkincil

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
} PACKED idt_entry_t;

typedef struct {
    uint16_t limit;         // IDT size - 1
    uint32_t base;          // IDT base address
} PACKED idt_ptr_t;

typedef void (*isr_t)(uint32_t error_code);

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);
void idt_load(idt_ptr_t* ptr);
void register_interrupt_handler(uint8_t num, isr_t handler);

// code will continue but im so bored.

#endif // IDT_H