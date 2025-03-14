#include <kernel/net/network.h>
#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/pci.h>
#include <security/firewall.h>
#include <../src/drivers/network/rtl8139.h>

static struct network_device* current_device = NULL;

void network_init(void) {
    terminal_writestring("Network init started\n");
    
    struct pci_device* pci_dev = pci_find_network_card();
    if(pci_dev) {
        terminal_writestring("Network card found!\n");
        if (rtl8139_init(pci_dev) == 0) {
            terminal_writestring("RTL8139 driver loaded successfully.\n");
        } else {
            terminal_writestring("Driver not loaded!\n");
        }
    } else {
        terminal_writestring("Network card not found!\n");
    }
    
    terminal_writestring("Firewall init started...\n");
    firewall_init();
}

int network_send_packet(void* data, size_t length) {
    if (!current_device) {
        return -1;
    }
    
    uint32_t src_ip = current_device->ip_address;
    uint32_t dst_ip = *(uint32_t*)((char*)data + 16);
    uint16_t src_port = *(uint16_t*)((char*)data + 20);
    uint16_t dst_port = *(uint16_t*)((char*)data + 22);
    uint8_t protocol = *(uint8_t*)((char*)data + 9);
    
    if (firewall_check_packet(src_ip, dst_ip, src_port, dst_port, protocol) == RULE_DENY) {
        terminal_writestring("Packet blocked by firewall.\n");
        return -2;
    }
    
    return current_device->send(current_device, data, length);
}

int network_receive_packet(void* buffer, size_t length) {
    if (!current_device) {
        return -1;
    }
    

    if (!buffer || length < 14) { 
        return -2;
    }
    
    uint8_t* eth_packet = (uint8_t*)buffer;
    
    uint16_t eth_type = (eth_packet[12] << 8) | eth_packet[13];
    
    if (eth_type != 0x0800 && eth_type != 0x0806) {
        return -3;
    }
    
    if (eth_type == 0x0800 && length >= 34) { 
        uint8_t* ip_header = eth_packet + 14;
        uint8_t ip_version = (ip_header[0] >> 4) & 0x0F;
        
        if (ip_version != 4) {
            return -4;
        }
        
        // security note: more packet validation can be done
        // - source ip check
        // - destination ip check
        // - ip header checksum validation
        // - protocol check (tcp/udp/icmp/...) 
    }
    
    return current_device->receive(current_device, buffer, length);
} 