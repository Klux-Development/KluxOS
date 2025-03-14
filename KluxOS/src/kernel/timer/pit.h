#ifndef PIT_H
#define PIT_H

#include <kernel/types.h>


#define PIT_CHANNEL0    0x40    // 0. channel data port
#define PIT_CHANNEL1    0x41    // 1st channel data port
#define PIT_CHANNEL2    0x42    // 2nd channel data port
#define PIT_COMMAND     0x43    // command/mode port

// PIT commands
#define PIT_CHANNEL0_SELECT  0x00    // channel 0 selection
#define PIT_CHANNEL1_SELECT  0x40    // channel 1 selection
#define PIT_CHANNEL2_SELECT  0x80    // channel 2 selection
#define PIT_READBACK        0xC0    // read command

// PIT working modes
#define PIT_MODE0           0x00    // interrupt counter
#define PIT_MODE1           0x02    // programable one-shot
#define PIT_MODE2           0x04    // rate generator
#define PIT_MODE3           0x06    // square wave generator
#define PIT_MODE4           0x08    // software triggered strobe
#define PIT_MODE5           0x0A    // hardware triggered strobe

// PIT data format
#define PIT_BINARY          0x00    // binary counting
#define PIT_BCD             0x01    // BCD (decimal) counting

// PIT access mode
#define PIT_LATCH           0x00    // lock current count
#define PIT_LOBYTE          0x10    // only low byte
#define PIT_HIBYTE          0x20    // only high byte
#define PIT_BOTH            0x30    // first low then high byte

// PIT frequency
#define PIT_FREQUENCY       1193180  // PIT's base frequency (Hz)

typedef struct {
    uint32_t ticks;          // number of ticks since system start
    uint32_t frequency;      // current timer frequency (Hz)
    uint32_t ms_per_tick;    // milliseconds per tick
    uint64_t uptime_ms;      // total uptime (milliseconds)
} timer_info_t;

void pit_init(uint32_t frequency);

void pit_tick(void);

uint32_t get_ticks(void);


uint64_t get_uptime_ms(void);
void sleep_ms(uint32_t ms);


typedef void (*timer_callback_t)(void);


void register_timer_callback(timer_callback_t callback);

timer_info_t* get_timer_info(void);

#endif // PIT_H 