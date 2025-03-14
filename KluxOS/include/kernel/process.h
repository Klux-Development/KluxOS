#ifndef PROCESS_H
#define PROCESS_H

#include <kernel/types.h>

// İşlem durumları
typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

// İşlem bağlam yapısı (context) - CPU durumu
typedef struct {
    uint32_t eax, ebx, ecx, edx;  // Genel amaçlı yazmaçlar
    uint32_t esi, edi, ebp, esp;  // İşaretçi yazmaçlar
    uint32_t eip;                 // Talimat işaretçisi
    uint32_t eflags;              // CPU bayrakları
    uint32_t cr3;                 // Sayfa dizini tabanı
} process_context_t;

// İşlem yapısı
typedef struct process {
    uint32_t pid;                 // İşlem kimliği
    char name[32];                // İşlem adı
    process_state_t state;        // İşlem durumu
    process_context_t context;    // İşlem bağlamı
    void* kernel_stack;           // Çekirdek yığını
    uint32_t kernel_stack_size;   // Çekirdek yığını boyutu
    void* user_stack;             // Kullanıcı yığını
    uint32_t user_stack_size;     // Kullanıcı yığını boyutu
    struct process* next;         // Sonraki işlem
} process_t;

// Aktif işlemler listesi
extern process_t* current_process;
extern process_t* process_list;

// İşlem yönetim fonksiyonları
void process_init(void);
process_t* process_create(const char* name, void* entry_point);
void process_terminate(process_t* process);
void process_switch(process_t* next);
void process_schedule(void);
process_t* process_get_current(void);

// Basit bir zamanlayıcı
void scheduler_init(void);
void scheduler_tick(void);

#endif
