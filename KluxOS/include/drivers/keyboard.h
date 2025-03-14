#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <kernel/types.h>
#include <drivers/terminal.h>

// bool tanımı
#ifndef __bool_true_false_are_defined
typedef enum { false = 0, true = 1 } bool;
#define __bool_true_false_are_defined 1
#endif

// PS/2 klavye port numaraları
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_COMMAND_PORT 0x64

// Klavye durum bayrağı
#define KEYBOARD_OUTPUT_BUFFER_FULL 0x01

// Özel tuş kodları
#define KEYCODE_ESCAPE 0x01
#define KEYCODE_BACKSPACE 0x0E
#define KEYCODE_TAB 0x0F
#define KEYCODE_ENTER 0x1C
#define KEYCODE_LCTRL 0x1D
#define KEYCODE_LSHIFT 0x2A
#define KEYCODE_RSHIFT 0x36
#define KEYCODE_LALT 0x38
#define KEYCODE_CAPSLOCK 0x3A
#define KEYCODE_F1 0x3B
#define KEYCODE_F2 0x3C
#define KEYCODE_F3 0x3D
#define KEYCODE_F4 0x3E
#define KEYCODE_F5 0x3F
#define KEYCODE_F6 0x40
#define KEYCODE_F7 0x41
#define KEYCODE_F8 0x42
#define KEYCODE_F9 0x43
#define KEYCODE_F10 0x44
#define KEYCODE_NUMLOCK 0x45
#define KEYCODE_SCROLLLOCK 0x46
#define KEYCODE_HOME 0x47
#define KEYCODE_UP 0x48
#define KEYCODE_PGUP 0x49
#define KEYCODE_LEFT 0x4B
#define KEYCODE_RIGHT 0x4D
#define KEYCODE_END 0x4F
#define KEYCODE_DOWN 0x50
#define KEYCODE_PGDN 0x51
#define KEYCODE_INSERT 0x52
#define KEYCODE_DELETE 0x53
#define KEYCODE_F11 0x57
#define KEYCODE_F12 0x58

// Klavye çalışma modları
typedef enum {
    KEYBOARD_MODE_RAW,      // Ham tuş kodları
    KEYBOARD_MODE_ASCII,    // ASCII karakterler
    KEYBOARD_MODE_ECHO      // ASCII + ekrana yazdırma
} keyboard_mode_t;

// Klavye düzeni
typedef enum {
    KEYBOARD_LAYOUT_US,     // US klavye düzeni
    KEYBOARD_LAYOUT_TR      // Türkçe Q klavye düzeni
} keyboard_layout_t;

// Tuş durumu bayrağı
typedef struct {
    bool shift_pressed;
    bool ctrl_pressed;
    bool alt_pressed;
    bool capslock_on;
    bool numlock_on;
    bool scrolllock_on;
} key_state_t;

// Karakter tamponu
#define KEYBOARD_BUFFER_SIZE 256

// Klavye başlatma
void keyboard_init(void);

// Klavye kesme işleyicisi
void keyboard_handler(void);

// Klavye modu ayarla
void keyboard_set_mode(keyboard_mode_t mode);

// Klavye düzeni ayarla
void keyboard_set_layout(keyboard_layout_t layout);

// Karakter oku (bloke edici)
char keyboard_getchar(void);

// Tampondan karakter al (bloke edici değil)
char keyboard_read(void);

// Tampon durumu
bool keyboard_data_available(void);

// Terminal okuma fonksiyonlarını bağla
void keyboard_connect_terminal(void);

#endif
