// ağ sürücüsü için gerekli olan sikim sonik dosyanın devamı
#include "rtl8139.h"
#include <kernel/kernel.h>
#include <kernel/pci.h>

static struct network_device rtl8139_dev;
static uint8_t rx_buffer[8192 + 16];
static uint32_t current_packet_ptr;

int rtl8139_init(struct pci_device* pci_dev) {
    if (pci_dev->vendor_id != RTL8139_VENDOR_ID || 
        pci_dev->device_id != RTL8139_DEVICE_ID) {
        return -1;
    }

    outb(RTL8139_PORT_CMD, 0x10);  // Soft reset
    while ((inb(RTL8139_PORT_CMD) & 0x10) != 0); // Reset tamamlanana kadar bekle

    outl(RTL8139_PORT_RBSTART, (uint32_t)rx_buffer);
    current_packet_ptr = 0;

    for (int i = 0; i < 6; i++) {
        rtl8139_dev.mac_address[i] = inb(RTL8139_PORT_MAC + i);
    }

    rtl8139_dev.send = rtl8139_send;
    rtl8139_dev.receive = rtl8139_receive;
    
    outw(RTL8139_PORT_CMD, 0x000C);  // Enable Tx/Rx

    current_device = &rtl8139_dev;
    return 0;
} 