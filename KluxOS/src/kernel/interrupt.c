#include <kernel/interrupt.h>
#include <kernel/types.h>
#include <drivers/terminal.h>
#include <compat.h> 

// Assembly makrosu tanımı compat.h'de 
// #define __asm__ asm

// Kesme yapıları
// #define IDT_ENTRIES 256  

// Kesme işleyici fonksiyon tipi tanımı
// typedef void (*isr_t)(void);  

// IDT giriş yapısı
// typedef struct {
//    uint16_t base_low;    
//    uint16_t selector;    
//    uint8_t  zero;        
//    uint8_t  flags;       
//    uint16_t base_high;   
// } idt_entry_t;

// IDT işaretçi yapısı
// typedef struct {
//    uint16_t limit;       
//    uint32_t base;        
// } idt_ptr_t;

static isr_t interrupt_handlers[IDT_ENTRIES];

extern void isr0(void);
extern void isr1(void);
extern void irq0(void);
extern void irq1(void);

static idt_entry_t idt_entries[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

static inline void load_idt(idt_ptr_t* ptr) {
    ASM_INLINE_INPUT1("lidt (%0)", "r" (ptr));
}

static inline void enable_interrupts(void) {
    ASM_INLINE("sti");
}

static inline void outb(uint8_t value, uint16_t port) {
    ASM_INLINE_INPUT2("outb %0, %1", "a"(value), "Nd"(port));
}

void init_idt(void) {
    terminal_writestring("Kesme sistemi baslatiliyor...\n");
    
    idt_ptr.limit = sizeof(idt_entry_t) * IDT_ENTRIES - 1;
    idt_ptr.base = (uint32_t)&idt_entries;
    
    for (int i = 0; i < IDT_ENTRIES; i++) { 
        set_idt_gate(i, 0, 0, 0);
        interrupt_handlers[i] = 0;
    }
    
    set_idt_gate(0, (uint32_t)isr0, 0x08, 0x8E); // Division by zero
    set_idt_gate(1, (uint32_t)isr1, 0x08, 0x8E); // Debug
    
    set_idt_gate(32, (uint32_t)irq0, 0x08, 0x8E); // Timer
    set_idt_gate(33, (uint32_t)irq1, 0x08, 0x8E); // Keyboard
    
    load_idt(&idt_ptr);
    
    enable_interrupts();
    
    terminal_writestring("Kesme sistemi baslatildi.\n");
}

void set_idt_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector = selector;
    idt_entries[num].zero = 0;
    idt_entries[num].flags = flags;
}

void register_interrupt_handler(uint8_t num, isr_t handler) {
    interrupt_handlers[num] = handler;
}

void isr_handler(uint32_t int_no) {
    if (interrupt_handlers[int_no] != 0) {
        interrupt_handlers[int_no]();
    } else {
        terminal_writestring("Islenmeyen kesme: ");
        char s[4];
        int i = 0;
        int n = int_no;
        do {
            s[i++] = n % 10 + '0';
            n /= 10;
        } while (n > 0);
        s[i] = '\0';
        
        for (int j = 0; j < i / 2; j++) {
            char tmp = s[j];
            s[j] = s[i - j - 1];
            s[i - j - 1] = tmp;
        }
        
        terminal_writestring(s);
        terminal_writestring("\n");
    }
}

void irq_handler(uint32_t irq_no) {
    if (irq_no >= 8) {
        outb(0x20, 0xA0);
    }
    outb(0x20, 0x20);
    
    if (interrupt_handlers[irq_no + 32] != 0) {
        interrupt_handlers[irq_no + 32]();
    }
}

void irq_timer_handler(void) {
    // Zamanlayıcı kesmesi işlemleri (ileride eklenecek)
}

void irq_keyboard_handler(void) {
    // Klavye kesmesi işlemleri (ileride eklenecek)
}
