#include <security/firewall.h>
#include <kernel/types.h>
#include <drivers/terminal.h>

#define MAX_RULES 256

static struct firewall_rule rules[MAX_RULES];
static int rule_count = 0;
static int firewall_enabled = 1;

void firewall_init(void) {
    rule_count = 0;
    firewall_enabled = 1;
    terminal_writestring("Firewall initialized.\n");
}

int firewall_add_rule(struct firewall_rule* rule) {
    if (rule_count >= MAX_RULES) {
        return -1;
    }
    
    rules[rule_count++] = *rule;
    return 0;
}

int firewall_check_packet(uint32_t src_ip, uint32_t dst_ip, 
                         uint16_t src_port, uint16_t dst_port, 
                         uint8_t protocol) {
    if (!firewall_enabled) {
        return RULE_ALLOW;
    }
    
    for (int i = 0; i < rule_count; i++) {
        if ((rules[i].src_ip == src_ip || rules[i].src_ip == 0) &&
            (rules[i].dst_ip == dst_ip || rules[i].dst_ip == 0) &&
            (rules[i].src_port == src_port || rules[i].src_port == 0) &&
            (rules[i].dst_port == dst_port || rules[i].dst_port == 0) &&
            (rules[i].protocol == protocol || rules[i].protocol == 0)) {
            return rules[i].action;
        }
    }
    
    return RULE_DENY;
}

void firewall_enable(void) {
    firewall_enabled = 1;
    terminal_writestring("Firewall enabled.\n");
}

void firewall_disable(void) {
    firewall_enabled = 0;
    terminal_writestring("Firewall disabled.\n");
} 