#ifndef _FIREWALL_H
#define _FIREWALL_H

#include <kernel/types.h>

// Paket filtreleme kuralları
#define RULE_ALLOW 1
#define RULE_DENY  0

struct firewall_rule {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;
    uint8_t action;
};

// Firewall fonksiyonları
void firewall_init(void);
int firewall_add_rule(struct firewall_rule* rule);
int firewall_check_packet(uint32_t src_ip, uint32_t dst_ip, 
                         uint16_t src_port, uint16_t dst_port, 
                         uint8_t protocol);
void firewall_enable(void);
void firewall_disable(void);

#endif 