#ifndef _NETWORK_H
#define _NETWORK_H

#include <kernel/types.h>
#include <stddef.h>

// typedef unsigned int size_t; // böyle gide gide bilgisayarı yok edecem amk

// Ağ kartı yapısı
struct network_device {
    char name[32];
    uint8_t mac_address[6];
    uint32_t ip_address;
    uint32_t netmask;
    uint32_t gateway;
    
    // Donanım fonksiyonları
    int (*send)(struct network_device*, void* data, size_t length);
    int (*receive)(struct network_device*, void* buffer, size_t length);
};

// Global ağ cihazı
extern struct network_device* current_device;

// Temel ağ fonksiyonları
void network_init(void);
int network_send_packet(void* data, size_t length);
int network_receive_packet(void* buffer, size_t length);

// IP protokol fonksiyonları
int ip_send_packet(uint32_t dest_ip, void* data, size_t length);
int ip_receive_packet(void* buffer, size_t length);

#endif 
