#ifndef _TERMINAL_H
#define _TERMINAL_H

#include <kernel/types.h>  // size_t ve bool tipleri burada tanımlı

// Terminal renk kodları
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

// Renk oluşturma yardımcı fonksiyonu
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

// Özel tuş kodları
typedef enum {
    KEY_NULL = 0,
    KEY_BACKSPACE = 8,
    KEY_TAB = 9,
    KEY_ENTER = 13,
    KEY_ESCAPE = 27,
    KEY_ARROW_UP = 128,
    KEY_ARROW_DOWN = 129,
    KEY_ARROW_LEFT = 130,
    KEY_ARROW_RIGHT = 131,
    KEY_DELETE = 132,
    KEY_HOME = 133,
    KEY_END = 134,
    KEY_PAGE_UP = 135,
    KEY_PAGE_DOWN = 136
} special_key_t;

// Imleç pozisyonu
typedef struct {
    uint16_t x;
    uint16_t y;
} cursor_pos_t;

// Terminal boyutları
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define TAB_SIZE 4

// Temel terminal fonksiyonları
void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_clear(void);

// Gelişmiş terminal fonksiyonları
void terminal_newline(void);
void terminal_backspace(void);
void terminal_delete(void);
void terminal_tab(void);
void terminal_move_cursor(uint16_t x, uint16_t y);
cursor_pos_t terminal_get_cursor(void);
void terminal_set_cursor_visible(bool visible);
void terminal_scroll(int lines);
void terminal_clear_line(uint16_t line);
void terminal_clear_screen_from_cursor(void);
void terminal_clear_screen_to_cursor(void);

// Formatlı çıktı fonksiyonları
void terminal_printf(const char* format, ...);
void terminal_print_int(int value);
void terminal_print_uint(unsigned int value);
void terminal_print_hex(unsigned int value);

// Giriş fonksiyonları
char terminal_getchar(void);
void terminal_gets(char* buffer, size_t size);
int terminal_read_line(char* buffer, size_t size);

// Terminal renk yardımcı fonksiyonları
void terminal_set_fg_color(enum vga_color fg);
void terminal_set_bg_color(enum vga_color bg);
void terminal_reset_color(void);

// Terminal bilgileri
uint16_t terminal_get_width(void);
uint16_t terminal_get_height(void);

#endif 