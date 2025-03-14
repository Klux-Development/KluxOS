#ifndef _SHELL_H
#define _SHELL_H

#include <kernel/types.h>

// Shell sürüm bilgisi
#define SHELL_VERSION "0.1.0"
#define SHELL_NAME "KluxShell"

// Komut hattı maksimum uzunluğu
#define SHELL_MAX_COMMAND_LENGTH 256

// Shell ayar bayrakları
typedef enum {
    SHELL_FLAG_ECHO = 1,          // Girilen komutları göster
    SHELL_FLAG_COLOR = 2,         // Renkli çıktı kullan
    SHELL_FLAG_VERBOSE = 4,       // Detaylı mod
    SHELL_FLAG_DEBUG = 8          // Hata ayıklama bilgilerini göster
} shell_flags_t;

// Shell durum kodu
typedef enum {
    SHELL_OK = 0,                 // Başarılı
    SHELL_ERROR_COMMAND_NOT_FOUND, // Komut bulunamadı
    SHELL_ERROR_INVALID_ARGUMENTS, // Geçersiz argümanlar
    SHELL_ERROR_PERMISSION_DENIED, // İzin hatası
    SHELL_ERROR_INTERNAL,         // İç hata
    SHELL_EXIT                    // Çıkış talebi
} shell_status_t;

// Komut yapısı
typedef struct {
    const char* name;                                     // Komut adı
    const char* description;                              // Komut açıklaması
    shell_status_t (*handler)(int argc, char** argv);     // Komut işleyici
    const char* usage;                                    // Kullanım bilgisi
} shell_command_t;

// Shell durumu
typedef struct {
    char current_dir[256];         // Mevcut dizin
    uint32_t flags;                // Shell bayrakları
    char username[32];             // Kullanıcı adı
    char hostname[32];             // Hostname
    char last_command[SHELL_MAX_COMMAND_LENGTH]; // Son komut
    int exit_code;                 // Son çıkış kodu
} shell_context_t;

// Shell başlatma
void shell_init(void);

// Shell döngüsü
void shell_run(void);

// Komut işleme
shell_status_t shell_execute_command(const char* command_line);

// Komut ayrıştırma
int shell_parse_args(char* command_line, char** argv, int max_args);

// Özel fonksiyonlar
void shell_print_prompt(void);
void shell_clear_screen(void);
void shell_print_banner(void);
void shell_help(void);

// Komut işleyicileri (temel komutlar)
shell_status_t cmd_help(int argc, char** argv);
shell_status_t cmd_clear(int argc, char** argv);
shell_status_t cmd_echo(int argc, char** argv);
shell_status_t cmd_exit(int argc, char** argv);
shell_status_t cmd_version(int argc, char** argv);
shell_status_t cmd_ls(int argc, char** argv);
shell_status_t cmd_cd(int argc, char** argv);
shell_status_t cmd_cat(int argc, char** argv);
shell_status_t cmd_mkdir(int argc, char** argv);
shell_status_t cmd_rm(int argc, char** argv);
shell_status_t cmd_ps(int argc, char** argv);
shell_status_t cmd_kill(int argc, char** argv);
shell_status_t cmd_uptime(int argc, char** argv);
shell_status_t cmd_date(int argc, char** argv);
shell_status_t cmd_shutdown(int argc, char** argv);
shell_status_t cmd_reboot(int argc, char** argv);
shell_status_t cmd_meminfo(int argc, char** argv);

#endif
