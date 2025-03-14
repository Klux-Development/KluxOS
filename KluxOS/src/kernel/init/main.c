#include <kernel/kernel.h>
#include <kernel/types.h>
#include <kernel/interrupt/idt.h>
#include <kernel/timer/pit.h>
#include <drivers/terminal.h>
#include <drivers/keyboard.h>
#include <shell/shell.h>
#include <security/security.h>
#include "../mm/memory.h"
#include "multiboot.h"
#include <compat.h>


// #define __asm__ asm


#define KLUXOS_VERSION "0.1.0"
#define KLUXOS_NAME "KluxOS"


static struct {
    uint32_t mem_lower;              // Lower memory amount (KB)
    uint32_t mem_upper;              // Upper memory amount (KB)
    uint32_t boot_device;            // Boot device
    char*    cmdline;                // Command line parameters
    uint32_t mods_count;             // Module count
    uint32_t mods_addr;              // Module address
    uint32_t mmap_length;            // Memory map length
    uint32_t mmap_addr;              // Memory map address
    char*    boot_loader_name;       // Bootloader name
    uint32_t vbe_control_info;       // VBE control information
    uint32_t vbe_mode_info;          // VBE mode information
    uint32_t vbe_mode;               // VBE mode
    uint32_t vbe_interface_seg;      // VBE interface segment
    uint32_t vbe_interface_off;      // VBE interface offset
    uint32_t vbe_interface_len;      // VBE interface length
} sys_info;

static int init_complete = 0;

static void timer_callback() {
    static uint32_t seconds = 0;
    static uint32_t last_tick = 0;
    uint32_t current_tick = get_ticks();
    
    if (current_tick - last_tick >= get_timer_info()->frequency) {
        seconds++;
        last_tick = current_tick;
        
        if (init_complete) {
            terminal_set_cursor_position(70, 0);
            terminal_set_fg_color(VGA_COLOR_LIGHT_GREEN);
            terminal_writestring("Uptime: ");
            terminal_print_int(seconds);
            terminal_writestring("s ");
            terminal_reset_color();
        }
    }
}


void kernel_main(void* multiboot_struct) {
    uint32_t magic = 0x2BADB002; // Multiboot1 Magic Offset
    
    
    sys_info.mem_lower = 640;                   // Typical value: 640 KB
    sys_info.mem_upper = 64 * 1024;             // Example: 64 MB
    sys_info.boot_loader_name = "GRUB";
    sys_info.cmdline = "root=/dev/sda1";
    
    // Terminal sürücüsünü başlat
    terminal_initialize();
    
    // Hoş geldiniz mesajını yazdır
    terminal_set_fg_color(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("\n");
    terminal_writestring("  K   K L     U   U X   X   OOO   SSSS \n");
    terminal_writestring("  K  K  L     U   U  X X   O   O S     \n");
    terminal_writestring("  KKK   L     U   U   X    O   O  SSS  \n");
    terminal_writestring("  K  K  L     U   U  X X   O   O     S \n");
    terminal_writestring("  K   K LLLLL  UUU  X   X   OOO  SSSS  \n");
    terminal_writestring("\n");
    terminal_set_fg_color(VGA_COLOR_WHITE);
    terminal_writestring(" ");
    terminal_writestring(KLUXOS_NAME);
    terminal_writestring(" ");
    terminal_writestring(KLUXOS_VERSION);
    terminal_writestring(" - Basit x86 Isletim Sistemi\n");
    terminal_writestring(" (c) 2023-2024 KluxOS Gelistirme Ekibi\n\n");
    terminal_reset_color();
    
    init_memory();
    

    idt_init();
    
    pit_init(100);

    register_timer_callback(timer_callback);
    
    keyboard_init();
    
    terminal_set_fg_color(VGA_COLOR_LIGHT_GREY);
    terminal_writestring("Sistem Bilgileri:\n");
    terminal_writestring("----------------\n");
    terminal_reset_color();
    
    terminal_writestring("Bootloader: ");
    terminal_writestring(sys_info.boot_loader_name);
    terminal_writestring("\n");
    
    terminal_writestring("Bellek: Alt: ");
    terminal_print_int(sys_info.mem_lower);
    terminal_writestring(" KB, Ust: ");
    terminal_print_int(sys_info.mem_upper);
    terminal_writestring(" KB\n");
    
    terminal_writestring("Komut Satiri: ");
    terminal_writestring(sys_info.cmdline);
    terminal_writestring("\n\n");
    
    memory_info();
    
    init_complete = 1;
    
    terminal_writestring("\nKabuğu başlatmak için herhangi bir tuşa basın...\n");
    keyboard_wait_keypress();
    
    shell_run();
    
    while(1) {
       
        ASM_INLINE("hlt"); 
    }
}  
