#include <security/audit.h>
#include <kernel/types.h>
#include <drivers/terminal.h>
#include "../kernel/mm/memory.h"

#define MAX_AUDIT_LOGS 1024

static audit_event_t* audit_logs = NULL;
static uint32_t audit_count = 0;
static uint32_t next_event_id = 1;
static audit_level_t current_level = AUDIT_LEVEL_INFO; // Varsayılan olarak INFO seviyesi

extern uint32_t get_tick_count(void);

void audit_init(void) {
    terminal_writestring("Denetim sistemi baslatiliyor...\n");
    
    audit_logs = (audit_event_t*)kmalloc(sizeof(audit_event_t) * MAX_AUDIT_LOGS);
    if (!audit_logs) {
        terminal_writestring("HATA: Denetim kayit tamponu icin bellek ayrilamadi!\n");
        return;
    }
    
    audit_count = 0;
    next_event_id = 1;
    
    audit_log(AUDIT_SYSTEM_BOOT, AUDIT_LEVEL_INFO, 0, "Sistem baslatildi");
    
    terminal_writestring("Denetim sistemi baslatildi.\n");
}

void audit_set_level(audit_level_t level) {
    if (level >= AUDIT_LEVEL_NONE && level <= AUDIT_LEVEL_VERBOSE) {
        current_level = level;
        
        char buffer[64];
        terminal_writestring("Denetim seviyesi ayarlandi: ");
        
        switch(level) {
            case AUDIT_LEVEL_NONE:
                terminal_writestring("NONE (Kapatildi)");
                break;
            case AUDIT_LEVEL_ERROR:
                terminal_writestring("ERROR (Sadece hatalar)");
                break;
            case AUDIT_LEVEL_WARNING:
                terminal_writestring("WARNING (Hatalar ve uyarilar)");
                break;
            case AUDIT_LEVEL_INFO:
                terminal_writestring("INFO (Bilgi, hatalar ve uyarilar)");
                break;
            case AUDIT_LEVEL_VERBOSE:
                terminal_writestring("VERBOSE (Tum olaylar)");
                break;
        }
        
        terminal_writestring("\n");
        
        audit_log(AUDIT_SECURITY_CHANGE, AUDIT_LEVEL_INFO, 0, "Denetim seviyesi degistirildi");
    }
}

int audit_log(audit_event_type_t event_type, audit_level_t level, uint32_t user_id, const char* message) {
    if (level > current_level || current_level == AUDIT_LEVEL_NONE) {
        return 0; 
    }
    
    if (!audit_logs) {
        return -1;
    }
    
    if (audit_count >= MAX_AUDIT_LOGS) {
        for (uint32_t i = 1; i < MAX_AUDIT_LOGS; i++) {
            audit_logs[i-1] = audit_logs[i];
        }
        audit_count = MAX_AUDIT_LOGS - 1;
    }
    
    audit_event_t* event = &audit_logs[audit_count];
    event->event_id = next_event_id++;
    event->timestamp = get_tick_count();
    event->type = event_type;
    event->level = level;
    event->user_id = user_id;
    
    int i;
    for (i = 0; message[i] != '\0' && i < 255; i++) {
        event->message[i] = message[i];
    }
    event->message[i] = '\0';
    
    audit_count++;
    
    return 0;
}

int audit_view_logs(uint32_t start_id, uint32_t count) {
    if (!audit_logs || audit_count == 0) {
        terminal_writestring("Denetim kaydi bulunamadi.\n");
        return -1;
    }
    
    terminal_writestring("ID  | ZAMAN   | TUR  | SEVIYE | KULLANICI | MESAJ\n");
    terminal_writestring("----+---------+------+--------+-----------+------------------------\n");
    
    uint32_t displayed = 0;
    
    for (uint32_t i = 0; i < audit_count && displayed < count; i++) {
        audit_event_t* event = &audit_logs[i];
        
        if (event->event_id < start_id) {
            continue;
        }
        
        const char* type_str = "OTHER";
        switch(event->type) {
            case AUDIT_LOGIN: type_str = "LOGIN"; break;
            case AUDIT_LOGOUT: type_str = "LOGOUT"; break;
            case AUDIT_FILE_ACCESS: type_str = "FILE"; break;
            case AUDIT_PROCESS_CREATE: type_str = "PROC+"; break;
            case AUDIT_PROCESS_EXIT: type_str = "PROC-"; break;
            case AUDIT_NETWORK_ACCESS: type_str = "NET"; break;
            case AUDIT_SECURITY_CHANGE: type_str = "SEC"; break;
            case AUDIT_SYSTEM_BOOT: type_str = "BOOT"; break;
            case AUDIT_SYSTEM_SHUTDOWN: type_str = "SHTDWN"; break;
            case AUDIT_USER_ADMIN: type_str = "ADMN"; break;
        }
        
        const char* level_str = "?";
        switch(event->level) {
            case AUDIT_LEVEL_ERROR: level_str = "ERROR"; break;
            case AUDIT_LEVEL_WARNING: level_str = "WARN"; break;
            case AUDIT_LEVEL_INFO: level_str = "INFO"; break;
            case AUDIT_LEVEL_VERBOSE: level_str = "VERB"; break;
        }
        
        terminal_printf("%3d | %7d | %4s | %6s | %9d | %s\n",
                       event->event_id,
                       event->timestamp,
                       type_str,
                       level_str,
                       event->user_id,
                       event->message);
        
        displayed++;
    }
    
    if (displayed == 0) {
        terminal_writestring("No audit records found matching the specified criteria.\n");
        return -1;
    }
    
    return displayed;
}

int audit_find_events(audit_event_type_t type, uint32_t user_id, uint32_t* results, uint32_t max_results) {
    if (!audit_logs || audit_count == 0 || !results) {
        return -1;
    }
    
    uint32_t found = 0;
    
    for (uint32_t i = 0; i < audit_count && found < max_results; i++) {
        audit_event_t* event = &audit_logs[i];
        
        if ((type == 0 || event->type == type) && 
            (user_id == 0 || event->user_id == user_id)) {
            
            results[found++] = event->event_id;
        }
    }
    
    return found;
}
