#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <kernel/types.h>
#include <compat.h>

#define IDT_ENTRIES 256

// IDT giriş yapısı
typedef struct {
    uint16_t base_low;        // Alt 16 bit
    uint16_t selector;        // Kod segmenti seçici
    uint8_t zero;             // Sabit 0
    uint8_t flags;            // Bayraklar
    uint16_t base_high;       // Üst 16 bit
} PACKED idt_entry_t;

// IDT işaretçisi
typedef struct {
    uint16_t limit;
    uint32_t base;
} PACKED idt_ptr_t;

// Kesme işleyici fonksiyon tipi
typedef void (*isr_t)(void);

// Fonksiyonlar
void init_idt(void);
void set_idt_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);
void register_interrupt_handler(uint8_t num, isr_t handler);

// Varsayılan kesme işleyicileri deklarasyonları
void irq_timer_handler(void);
void irq_keyboard_handler(void);

#endif
