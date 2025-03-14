#ifndef _KERNEL_H
#define _KERNEL_H

#include <kernel/types.h>
#include <kernel/io.h>

#define KERNEL_VERSION "0.0.1"

void kernel_panic(const char* message);

// Temel I/O fonksiyonları - artık io.h dosyasinda
// void outb(uint16_t port, uint8_t value);
// uint8_t inb(uint16_t port);
// void io_wait(void);

// Kesme yönetimi
void cli(void);  // Kesmeleri devre dışı bırak
void sti(void);  // Kesmeleri etkinleştir
void hlt(void);  // CPU'yu durdur

// Sistem kontrolü
void reboot(void);
void shutdown(void);

// Debug fonksiyonları
#ifdef DEBUG
    #define KDEBUG(msg) kernel_debug(msg)
    void kernel_debug(const char* message);
#else
    #define KDEBUG(msg)
#endif

#endif 