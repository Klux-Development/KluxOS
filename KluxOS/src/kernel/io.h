#ifndef IO_H
#define IO_H

#include <kernel/types.h>

// Port I/O işlemleri için inline fonksiyonlar

// Tek byte veri yazma (8-bit)
static inline void outb(uint16_t port, uint8_t value) {
#if defined(__GNUC__) || defined(__clang__)
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
#else
    // Başka derleyiciler için alternatif implementasyon
    // Not: Gerçek bir işletim sisteminde bu işlem her zaman assembly gerektirir
#endif
}

// Tek byte veri okuma (8-bit)
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
#if defined(__GNUC__) || defined(__clang__)
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
#else
    // Başka derleyiciler için alternatif implementasyon
    ret = 0; // Varsayılan değer
#endif
    return ret;
}

// Word veri yazma (16-bit)
static inline void outw(uint16_t port, uint16_t value) {
#if defined(__GNUC__) || defined(__clang__)
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
#else
    // Başka derleyiciler için alternatif implementasyon
#endif
}

// Word veri okuma (16-bit)
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
#if defined(__GNUC__) || defined(__clang__)
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
#else
    // Başka derleyiciler için alternatif implementasyon
    ret = 0; // Varsayılan değer
#endif
    return ret;
}

// Double word veri yazma (32-bit)
static inline void outl(uint16_t port, uint32_t value) {
#if defined(__GNUC__) || defined(__clang__)
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
#else
    // Başka derleyiciler için alternatif implementasyon
#endif
}

// Double word veri okuma (32-bit)
static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
#if defined(__GNUC__) || defined(__clang__)
    __asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
#else
    // Başka derleyiciler için alternatif implementasyon
    ret = 0; // Varsayılan değer
#endif
    return ret;
}

// I/O işlemleri arasında kısa gecikme için
static inline void io_wait(void) {
    // 0x80 portu genellikle kullanılmayan bir port
    // Bu portu kullanmak işlemi geciktirir
    outb(0x80, 0);
}

#endif // IO_H 