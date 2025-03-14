#ifndef PIC_H
#define PIC_H

#include <kernel/types.h>

#define PIC1         0x20    // primary PIC
#define PIC2         0xA0    // secondary PIC
#define PIC1_COMMAND PIC1
#define PIC1_DATA    (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA    (PIC2+1)

#define PIC_EOI      0x20    // End of interrupt command
#define PIC_READ_IRR 0x0A    // Read pending interrupt request
#define PIC_READ_ISR 0x0B    // Read serviced interrupt

#define ICW1_ICW4    0x01    // ICW4 required
#define ICW1_SINGLE  0x02    // Single PIC
#define ICW1_INTERVAL4 0x04  // Use 4 instead of 8 byte interval
#define ICW1_LEVEL   0x08    // Level triggered mode
#define ICW1_INIT    0x10    // Initialize required

#define ICW4_8086    0x01    // 8086/88 modu
#define ICW4_AUTO    0x02    // Auto EOI
#define ICW4_BUF_SLAVE 0x08  // Buffer mode - Slave
#define ICW4_BUF_MASTER 0x0C // Buffer mode - Master
#define ICW4_SFNM    0x10    // Special fully nested mode

// IRQ tanımları
#define IRQ0  0
#define IRQ1  1
#define IRQ2  2
#define IRQ3  3
#define IRQ4  4
#define IRQ5  5
#define IRQ6  6
#define IRQ7  7
#define IRQ8  8
#define IRQ9  9
#define IRQ10 10
#define IRQ11 11
#define IRQ12 12
#define IRQ13 13
#define IRQ14 14
#define IRQ15 15

void pic_init(uint8_t offset1, uint8_t offset2);
void pic_send_eoi(uint8_t irq);
void pic_set_mask(uint8_t irq);
void pic_clear_mask(uint8_t irq);
uint16_t pic_get_irr(void);
uint16_t pic_get_isr(void);
void pic_disable(void);

#endif // PIC_H 