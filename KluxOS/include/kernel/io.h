#ifndef IO_H
#define IO_H

#include <kernel/types.h>
#include <compat.h>

// 8-bit port I/O işlemleri
static inline void outb(uint16_t port, uint8_t value) {
    ASM_INLINE_INPUT2("outb %0, %1", "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    ASM_INLINE_PARAMS("inb %1, %0", "=a"(ret), "Nd"(port));
    return ret;
}

// 16-bit port I/O işlemleri
static inline void outw(uint16_t port, uint16_t value) {
    ASM_INLINE_INPUT2("outw %0, %1", "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    ASM_INLINE_PARAMS("inw %1, %0", "=a"(ret), "Nd"(port));
    return ret;
}

// 32-bit port I/O işlemleri
static inline void outl(uint16_t port, uint32_t value) {
    ASM_INLINE_INPUT2("outl %0, %1", "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    ASM_INLINE_PARAMS("inl %1, %0", "=a"(ret), "Nd"(port));
    return ret;
}

// I/O işlemleri arasında kısa bir gecikme
static inline void io_wait(void) {
    // 0x80 portu, genellikle boştur ve sadece I/O gecikmeleri için kullanılır
    outb(0x80, 0);
}

#endif // IO_H 