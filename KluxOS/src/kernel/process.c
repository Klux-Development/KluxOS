#include <kernel/process.h>
#include <kernel/types.h>
#include <kernel/interrupt.h>
#include <drivers/terminal.h>
#include "../kernel/mm/memory.h"

static inline void cli(void) { /* Kesmeleri devre dışı bırak */ }
static inline void sti(void) { /* Kesmeleri etkinleştir */ }
static inline void save_context(process_t* proc) { /* İşlem bağlamını kaydet */ }
static inline void load_context(process_t* proc) { /* İşlem bağlamını yükle */ }
static inline void io_wait(void) { /* I/O beklemesi */ }
static inline void port_out(uint8_t value, uint16_t port) { /* Port I/O işlemi */ }

process_t* current_process = NULL;
process_t* process_list = NULL;
static uint32_t next_pid = 1;

void process_init(void) {
    terminal_writestring("Islem yonetimi baslatiliyor...\n");
    
    // İlk çalışan işlem kernel olacak (pid=0)
    process_t* kernel_process = (process_t*)kmalloc(sizeof(process_t));
    
    kernel_process->pid = 0;
    kernel_process->state = PROCESS_RUNNING;
    
    int i;
    const char* name = "kernel";
    for (i = 0; name[i] != '\0' && i < 31; i++) {
        kernel_process->name[i] = name[i];
    }
    kernel_process->name[i] = '\0';
    
    kernel_process->context.eip = 0;
    kernel_process->context.esp = 0;
    kernel_process->context.ebp = 0;
    
    kernel_process->kernel_stack_size = 4096;
    kernel_process->kernel_stack = kmalloc(kernel_process->kernel_stack_size);
    
    kernel_process->next = NULL;
    process_list = kernel_process;
    current_process = kernel_process;
    
    terminal_writestring("Islem yonetimi baslatildi.\n");
    
    scheduler_init();
}   

process_t* process_create(const char* name, void* entry_point) {
    terminal_writestring("Yeni islem olusturuluyor: ");
    terminal_writestring(name);
    terminal_writestring("\n");
    
    process_t* new_process = (process_t*)kmalloc(sizeof(process_t));
    
    new_process->pid = next_pid++;
    new_process->state = PROCESS_READY;
    
    int i;
    for (i = 0; name[i] != '\0' && i < 31; i++) {
        new_process->name[i] = name[i];
    }
    new_process->name[i] = '\0';
    
    new_process->context.eip = (uint32_t)entry_point;
    new_process->context.eflags = 0x202; // Kesmeler aktif
    
    new_process->kernel_stack_size = 4096;
    new_process->kernel_stack = kmalloc(new_process->kernel_stack_size);
    
    new_process->context.esp = (uint32_t)new_process->kernel_stack + new_process->kernel_stack_size;
    new_process->context.ebp = new_process->context.esp;
    
    new_process->next = process_list;
    process_list = new_process;
    
    return new_process;
}

void process_terminate(process_t* process) {
    if (!process) return;
    
    terminal_writestring("Islem sonlandiriliyor: ");
    terminal_writestring(process->name);
    terminal_writestring("\n");
    
    if (process == process_list) {
        process_list = process->next;
    } else {
        process_t* temp = process_list;
        while (temp && temp->next != process) {
            temp = temp->next;
        }
        
        if (temp) {
            temp->next = process->next;
        }
    }
    
    if (process == current_process) {
        current_process = process_list;
    }
    
    process->state = PROCESS_TERMINATED;
    
    if (process->kernel_stack) {
        kfree(process->kernel_stack);
    }
    
    kfree(process);
}

void process_switch(process_t* next) {
    if (!next || next == current_process) return;
    
    cli();
    
    save_context(current_process);
    
    current_process->state = PROCESS_READY;
    
    current_process = next;
    current_process->state = PROCESS_RUNNING;
    
    load_context(current_process);
    
    sti();
}

void process_schedule(void) {
    if (!process_list) return;
    
    process_t* next = current_process->next;
    
    if (!next) {
        next = process_list;
    }
    
    while (next != current_process) {
        if (next->state == PROCESS_READY) {
            break;
        }
        
        next = next->next;
        if (!next) {
            next = process_list;
        }
    }
    
    if (next != current_process && next->state == PROCESS_READY) {
        process_switch(next);
    }
}

process_t* process_get_current(void) {
    return current_process;
}

void scheduler_init(void) {
    terminal_writestring("Zamanlayici baslatiliyor...\n");
    
    register_interrupt_handler(32, (isr_t)scheduler_tick);
    
    terminal_writestring("Zamanlayici baslatildi.\n");
}

static uint32_t tick_count = 0;
static uint32_t time_slice = 10;

uint32_t get_tick_count(void) {
    return tick_count;
}

void ack_irq(int irq_no) {
    if (irq_no >= 8) {
        port_out(0x20, 0xA0);
    }
    port_out(0x20, 0x20);
}

void scheduler_tick(void) {
    ack_irq(0); // IRQ0 - timer 
    
    tick_count++;
    
    if (tick_count % time_slice == 0) {
        if (process_list && process_list->next) {
            process_schedule();
        }
    }
}
