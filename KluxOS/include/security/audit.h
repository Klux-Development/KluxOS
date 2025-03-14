#ifndef AUDIT_H
#define AUDIT_H

#include <kernel/types.h>

// Denetim olayı türleri
typedef enum {
    AUDIT_LOGIN = 1,             // Kullanıcı girişi
    AUDIT_LOGOUT = 2,            // Kullanıcı çıkışı
    AUDIT_FILE_ACCESS = 3,       // Dosya erişimi
    AUDIT_PROCESS_CREATE = 4,    // İşlem oluşturma
    AUDIT_PROCESS_EXIT = 5,      // İşlem sonlandırma
    AUDIT_NETWORK_ACCESS = 6,    // Ağ erişimi
    AUDIT_SECURITY_CHANGE = 7,   // Güvenlik ayarı değişikliği
    AUDIT_SYSTEM_BOOT = 8,       // Sistem açılışı
    AUDIT_SYSTEM_SHUTDOWN = 9,   // Sistem kapanışı
    AUDIT_USER_ADMIN = 10,       // Kullanıcı yönetimi
    AUDIT_OTHER = 255            // Diğer olaylar
} audit_event_type_t;

// Denetim kaydı seviyesi
typedef enum {
    AUDIT_LEVEL_NONE = 0,       // Denetim kaydı tutma
    AUDIT_LEVEL_ERROR = 1,      // Sadece hataları kaydet
    AUDIT_LEVEL_WARNING = 2,    // Hata ve uyarıları kaydet
    AUDIT_LEVEL_INFO = 3,       // Bilgi, hata ve uyarıları kaydet
    AUDIT_LEVEL_VERBOSE = 4     // Tüm olayları kaydet
} audit_level_t;

// Denetim kaydı oluşturma
int audit_log(audit_event_type_t event_type, 
              audit_level_t level, 
              uint32_t user_id, 
              const char* message);

// Denetim olayı yapısı
typedef struct {
    uint32_t event_id;           // Olay kimliği
    uint32_t timestamp;          // Zaman damgası
    audit_event_type_t type;     // Olay türü
    audit_level_t level;         // Olay seviyesi
    uint32_t user_id;            // Kullanıcı kimliği
    char message[256];           // Olay açıklaması
} audit_event_t;

// Denetim sistemi başlatma
void audit_init(void);

// Denetim seviyesi ayarlama
void audit_set_level(audit_level_t level);

// Denetim kaydı görüntüleme
int audit_view_logs(uint32_t start_id, uint32_t count);

// Belirli bir olayı ara
int audit_find_events(audit_event_type_t type, uint32_t user_id, uint32_t* results, uint32_t max_results);

#endif // AUDIT_H
