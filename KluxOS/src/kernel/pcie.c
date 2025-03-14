#include <kernel/pcie.h>
#include <kernel/kernel.h>
#include <drivers/terminal.h>
#include <kernel/io.h>  

static struct pcie_device* current_pcie_device = NULL;

uint32_t pcie_read_config(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = PCIE_CONFIG_ADDRESS | ((uint32_t)bus << 20) |
                      ((uint32_t)device << 15) | ((uint32_t)function << 12) | offset;
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

struct pcie_device* pcie_find_wifi_card(void) {
    static struct pcie_device dev;
    
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            uint32_t id = pcie_read_config(bus, device, 0, 0);
            
            if ((id & 0xFFFF) != 0xFFFF) {
                uint8_t class_code = pcie_read_config(bus, device, 0, 0x0B) & 0xFF;
                uint8_t subclass = pcie_read_config(bus, device, 0, 0x0A) & 0xFF;
                
                if (class_code == PCIE_CLASS_NETWORK && subclass == PCIE_SUBCLASS_WIFI) {
                    dev.vendor_id = id & 0xFFFF;
                    dev.device_id = (id >> 16) & 0xFFFF;
                    dev.class_code = class_code;
                    dev.subclass = subclass;
                    
                    terminal_writestring("WiFi karti bulundu: ");
                    
                    switch(dev.vendor_id) {
                        case 0x8086: // Intel
                            terminal_writestring("Intel ");
                            
                            // Intel cihazları için device ID kontrolü
                            switch(dev.device_id) {
                                case 0x0042: terminal_writestring("Centrino Ultimate-N 6300"); break;
                                case 0x0085: terminal_writestring("Centrino Advanced-N 6205"); break;
                                case 0x0082: terminal_writestring("Centrino Advanced-N 6205"); break;
                                case 0x0091: terminal_writestring("Wireless-AC 7260"); break;
                                case 0x2526: terminal_writestring("Wireless-AC 9260"); break;
                                case 0x2723: terminal_writestring("Wireless-AC 9560"); break;
                                case 0x3165: terminal_writestring("Wireless-AC 3165"); break;
                                case 0x08b1: terminal_writestring("Wireless-AC 7265"); break;
                                case 0x095a: terminal_writestring("Wireless-AC 9560"); break;
                                default: terminal_printf("Wireless Kart (ID: 0x%04X)", dev.device_id);
                            }
                            break;
                        
                        case 0x168c: // Qualcomm Atheros
                            terminal_writestring("Qualcomm Atheros ");
                            
                            // Atheros cihazları için device ID kontrolü
                            switch(dev.device_id) {
                                case 0x0030: terminal_writestring("AR93xx"); break;
                                case 0x0032: terminal_writestring("AR9485"); break;
                                case 0x0033: terminal_writestring("AR9580"); break;
                                case 0x0042: terminal_writestring("QCA9377"); break;
                                case 0x003e: terminal_writestring("QCA6174"); break;
                                default: terminal_printf("Wireless Kart (ID: 0x%04X)", dev.device_id);
                            }
                            break;
                            
                        case 0x14e4: // Broadcom
                            terminal_writestring("Broadcom ");
                            
                            // Broadcom cihazları için device ID kontrolü
                            switch(dev.device_id) {
                                case 0x4315: terminal_writestring("BCM4312"); break;
                                case 0x4357: terminal_writestring("BCM43225"); break;
                                case 0x4727: terminal_writestring("BCM4313"); break;
                                case 0x4353: terminal_writestring("BCM43224"); break;
                                case 0x432b: terminal_writestring("BCM4322"); break;
                                case 0x43a0: terminal_writestring("BCM4360"); break;
                                default: terminal_printf("Wireless Kart (ID: 0x%04X)", dev.device_id);
                            }
                            break;
                            
                        case 0x1814: // Ralink
                            terminal_writestring("Ralink/MediaTek ");
                            
                            // Ralink cihazları için device ID kontrolü
                            switch(dev.device_id) {
                                case 0x0601: terminal_writestring("RT2800"); break;
                                case 0x0781: terminal_writestring("RT2700/RT2800"); break;
                                case 0x3090: terminal_writestring("RT3090"); break;
                                case 0x3062: terminal_writestring("RT3062"); break;
                                case 0x539b: terminal_writestring("RT5390"); break;
                                default: terminal_printf("Wireless Kart (ID: 0x%04X)", dev.device_id);
                            }
                            break;
                            
                        default:
                            // Bilinmeyen üretici
                            terminal_printf("Bilinmeyen Uretici (ID: 0x%04X), Cihaz ID: 0x%04X", 
                                dev.vendor_id, dev.device_id);
                    }
                    
                    terminal_writestring("\n");
                    
                    return &dev;
                }
            }
        }
    }
    
    return NULL;
}
