#include "pit.h"
#include <kernel/interrupt/idt.h>
#include <kernel/io.h>
#include <drivers/terminal.h>
#include <compat.h>

static timer_info_t timer = {
    .ticks = 0,
    .frequency = 0,
    .ms_per_tick = 0,
    .uptime_ms = 0
};

static timer_callback_t timer_callback = NULL;

static void pit_handler(uint32_t error_code) {
    timer.ticks++;
    
    timer.uptime_ms += timer.ms_per_tick;
    
    if (timer_callback != NULL) {
        timer_callback();
    }
}

void pit_init(uint32_t frequency) {
    terminal_writestring("Initializing timer...\n");
    
    if (frequency < 18) frequency = 18;
    if (frequency > 1000) frequency = 1000;
    
    timer.frequency = frequency;
    timer.ms_per_tick = 1000 / frequency;
    
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    outb(PIT_COMMAND, PIT_CHANNEL0_SELECT | PIT_MODE3 | PIT_BOTH);
    
    outb(PIT_CHANNEL0, divisor & 0xFF);
    io_wait();
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
    
    register_interrupt_handler(IRQ0, pit_handler);
    
    terminal_writestring("  Frequency: ");
    terminal_print_int(frequency);
    terminal_writestring(" Hz\n");
    terminal_writestring("  Time slice: ");
    terminal_print_int(timer.ms_per_tick);
    terminal_writestring(" ms\n");
    
    terminal_writestring("Timer initialized.\n");
}

uint32_t get_ticks(void) {
    return timer.ticks;
}

uint64_t get_uptime_ms(void) {
    return timer.uptime_ms;
}

void sleep_ms(uint32_t ms) {
    uint32_t start = get_ticks();
    uint32_t ticks_to_wait = (ms + timer.ms_per_tick - 1) / timer.ms_per_tick;
    uint32_t end = start + ticks_to_wait;
    
    while (get_ticks() < end) {
        ASM_INLINE("hlt");
    }
}

void register_timer_callback(timer_callback_t callback) {
    timer_callback = callback;
}

timer_info_t* get_timer_info(void) {
    return &timer;
} 