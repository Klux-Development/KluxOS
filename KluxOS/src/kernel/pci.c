#include <kernel/pci.h>
#include <kernel/kernel.h>

uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (device << 11) |
                                 (func << 8) | (offset & 0xfc) | 0x80000000);
    
    outb(PCI_CONFIG_ADDRESS, address);
    return inb(PCI_CONFIG_DATA);
}

void pci_write(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = (uint32_t)((bus << 16) | (device << 11) |
                                 (func << 8) | (offset & 0xfc) | 0x80000000);
    
    outb(PCI_CONFIG_ADDRESS, address);
    outb(PCI_CONFIG_DATA, value);
}

struct pci_device* pci_find_network_card(void) {
    static struct pci_device dev;
    

    for(uint16_t bus = 0; bus < 256; bus++) {
        for(uint8_t device = 0; device < 32; device++) {
            uint32_t id = pci_read(bus, device, 0, 0);
            

            if((id & 0xFFFF) != 0xFFFF) {
                uint8_t class = pci_read(bus, device, 0, 0x0B);
                uint8_t subclass = pci_read(bus, device, 0, 0x0A);
                

                if(class == 0x02 && subclass == 0x00) {
                    dev.vendor_id = id & 0xFFFF;
                    dev.device_id = (id >> 16) & 0xFFFF;
                    return &dev;
                }
            }
        }
    }
    
    return NULL;
}