// ağ kartı sürücüsü için gerekli olan sikim sonik bir dosya.RTL
#ifndef _RTL8139_H
#define _RTL8139_H

#include <kernel/types.h>
#include <kernel/net/network.h>

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

// RTL8139 port adresleri
#define RTL8139_PORT_MAC     0x00
#define RTL8139_PORT_MAR     0x08
#define RTL8139_PORT_TSD     0x10
#define RTL8139_PORT_TSAD    0x20
#define RTL8139_PORT_RBSTART 0x30
#define RTL8139_PORT_CMD     0x37
#define RTL8139_PORT_CAPR    0x38
#define RTL8139_PORT_IMR     0x3C
#define RTL8139_PORT_ISR     0x3E
#define RTL8139_PORT_CONFIG  0x40

// Sürücü fonksiyonları
int rtl8139_init(struct pci_device* dev);
int rtl8139_send(struct network_device* dev, void* data, size_t length);
int rtl8139_receive(struct network_device* dev, void* buffer, size_t length);

#endif 