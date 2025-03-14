#include "pic.h"
#include <kernel/io.h>


static uint8_t pic1_mask = 0xFF;
static uint8_t pic2_mask = 0xFF;

static inline void io_wait(void) {
    outb(0x80, 0);
}

void pic_init(uint8_t offset1, uint8_t offset2) {
    pic1_mask = inb(PIC1_DATA);
    pic2_mask = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, offset1);  // primary PIC: IRQ 0-7 -> INT offset1..offset1+7
    io_wait();
    outb(PIC2_DATA, offset2);  // secondary PIC: IRQ 8-15 -> INT offset2..offset2+7
    io_wait();

    outb(PIC1_DATA, 4);        // primary PIC: IRQ2 secondary PIC'e connected (bit 2)
    io_wait();
    outb(PIC2_DATA, 2);        // secondary PIC: cascade id 2
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void pic_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
        pic1_mask |= (1 << irq);
        value = pic1_mask;
    } else {
        port = PIC2_DATA;
        pic2_mask |= (1 << (irq - 8));
        value = pic2_mask;
    }
    outb(port, value);
}

void pic_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
        pic1_mask &= ~(1 << irq);
        value = pic1_mask;
    } else {
        port = PIC2_DATA;
        pic2_mask &= ~(1 << (irq - 8));
        value = pic2_mask;
    }
    outb(port, value);
}

uint16_t pic_get_irr(void) {
    outb(PIC1_COMMAND, PIC_READ_IRR);
    outb(PIC2_COMMAND, PIC_READ_IRR);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

uint16_t pic_get_isr(void) {
    outb(PIC1_COMMAND, PIC_READ_ISR);
    outb(PIC2_COMMAND, PIC_READ_ISR);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

void pic_disable(void) {
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
} 