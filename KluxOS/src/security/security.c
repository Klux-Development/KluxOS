#include <kernel/kernel.h>
#include <kernel/types.h>
#include <security/aes.h>
#include <drivers/terminal.h>
#include <security/acl.h>

typedef unsigned int size_t;

extern void firewall_init(void);
extern int firewall_add_rule(int port, int action);
extern int firewall_check_packet(void* packet, size_t size);

extern void audit_init(void);
extern void audit_log(const char* message, uint32_t severity, uint32_t user_id);

extern void crypto_init(void);
extern int crypto_hash(const uint8_t* data, size_t length, uint8_t* hash);
extern int crypto_verify_signature(const uint8_t* data, size_t data_len, const uint8_t* signature, size_t sig_len);

typedef struct {
    uint8_t security_level;          // 0-4 (0: düşük, 4: yüksek)
    uint8_t enable_audit;            // Denetim kaydı aktif mi?
    uint8_t enable_firewall;         // Güvenlik duvarı aktif mi?
    uint8_t enable_access_control;   // Erişim kontrolü aktif mi?
    uint8_t enable_encryption;       // Şifreleme aktif mi?
} security_settings_t;

static security_settings_t security_config = {
    .security_level = 2,        
    .enable_audit = 1,          
    .enable_firewall = 1,       
    .enable_access_control = 1, 
    .enable_encryption = 1      
};

void security_init(void) {
    terminal_writestring("KluxOS güvenlik sistemi başlatılıyor...\n");
    
    terminal_writestring("AES şifreleme modülü başlatılıyor...\n");
    uint8_t key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
                       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    
    aes_context_t ctx;
    if (aes_init(&ctx, key, AES_128) == 0) {
        terminal_writestring("AES-128 başlatıldı.\n");
    } else {
        terminal_writestring("AES başlatılamadı!\n");
        security_config.enable_encryption = 0; // Şifreleme devre dışı
    }
    
    if (security_config.enable_access_control) {
        terminal_writestring("Erişim kontrol listesi (ACL) başlatılıyor...\n");
    }
    
    if (security_config.enable_audit) {
        terminal_writestring("Güvenlik denetim kaydı başlatılıyor...\n");
        audit_init();
        terminal_writestring("Güvenlik denetim kaydı başlatıldı.\n");
        
        audit_log("Güvenlik sistemi başlatıldı", 1, 0);
    }
    
    terminal_writestring("Şifreleme alt sistemi başlatılıyor...\n");
    crypto_init();
    terminal_writestring("Şifreleme alt sistemi başlatıldı.\n");
    
    if (security_config.enable_firewall) {
        terminal_writestring("Güvenlik duvarı başlatılıyor...\n");
        firewall_init();
        terminal_writestring("Güvenlik duvarı başlatıldı.\n");
        
        firewall_add_rule(80, 1);  // HTTP izin ver
        firewall_add_rule(443, 1); // HTTPS izin ver
        firewall_add_rule(22, 0);  // SSH engelle (güvenlik)
    }
    
    terminal_writestring("Güvenlik seviyesi: ");
    char level[2] = {'0' + security_config.security_level, '\0'};
    terminal_writestring(level);
    terminal_writestring("/4\n");
    
    terminal_writestring("Güvenlik sistemi başlatıldı.\n");
}

int security_set_level(uint8_t level) {
    if (level > 4) return -1; 
    
    security_config.security_level = level;
    
    if (level == 0) {
        security_config.enable_audit = 0;
        security_config.enable_firewall = 0;
        security_config.enable_access_control = 0;
        security_config.enable_encryption = 0;
    }
    else if (level == 1) {
        security_config.enable_audit = 1;
        security_config.enable_firewall = 0;
        security_config.enable_access_control = 0;
        security_config.enable_encryption = 1;
    }
    else if (level == 2) {
        security_config.enable_audit = 1;
        security_config.enable_firewall = 1;
        security_config.enable_access_control = 1;
        security_config.enable_encryption = 1;
    }
    else if (level == 3) {
        security_config.enable_audit = 1;
        security_config.enable_firewall = 1;
        security_config.enable_access_control = 1;
        security_config.enable_encryption = 1;
        firewall_add_rule(0, 0); 
    }
    else if (level == 4) {
        security_config.enable_audit = 1;
        security_config.enable_firewall = 1;
        security_config.enable_access_control = 1;
        security_config.enable_encryption = 1;
        firewall_add_rule(0, 0); 
        firewall_add_rule(443, 1); 
    }
    
    if (security_config.enable_audit) {
        char msg[64];
        terminal_sprintf(msg, "Güvenlik seviyesi değiştirildi: %d", level);
        audit_log(msg, 2, 0);
    }
    
    return 0;
}

int security_add_firewall_rule(int port, int action) {
    if (!security_config.enable_firewall) return -1;
    return firewall_add_rule(port, action);
}

int security_encrypt_data(const uint8_t* data, size_t length, uint8_t* output) {
    if (!security_config.enable_encryption) return -1;
    
    aes_context_t ctx;
    uint8_t key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
                       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    
    if (aes_init(&ctx, key, AES_128) != 0) {
        return -1;
    }
    
    // TODO: Gerçek implementasyonda IV (başlangıç vektörü) kullanılmalı
    // TODO: AES-CBC, AES-GCM gibi daha güvenli modlar kullanılmalı
    
    aes_encrypt_block(&ctx, data, output);
    
    return 0;
}

int security_decrypt_data(const uint8_t* data, size_t length, uint8_t* output) {
    if (!security_config.enable_encryption) return -1;
    
    aes_context_t ctx;
    uint8_t key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
                       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    
    if (aes_init(&ctx, key, AES_128) != 0) {
        return -1;
    }
    
    aes_decrypt_block(&ctx, data, output);
    // Şimdilik basit ECB modu kullanıyorum burda (güvenli değil, sadece örnek)
    aes_decrypt_block(&ctx, data, output);
    
    return 0;
}
