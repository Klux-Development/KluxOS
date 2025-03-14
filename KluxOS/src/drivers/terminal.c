#include <drivers/terminal.h>
#include <kernel/types.h>
#include <compat.h>  // Assembly uyumluluğu için eklendi

// VGA bellek başlangıç adresi
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

// Terminal durumu
static uint8_t terminal_color;
static uint16_t terminal_row;
static uint16_t terminal_column;
static bool cursor_visible = true;

// Karakter + özellik birleştirme
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

// Terminal başlatma
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    // Tüm terminali boşluk karakteri ile doldur
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', terminal_color);
        }
    }
    
    // Donanım imlecini ayarla
    terminal_set_cursor_visible(true);
    terminal_move_cursor(0, 0);
}

// Terminal rengini ayarla
void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

// Terminal pozisyonundaki girişi ayarla
static inline void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    VGA_MEMORY[index] = vga_entry(c, color);
}

// Yeni satır
void terminal_newline(void) {
    terminal_column = 0;
    if (++terminal_row == VGA_HEIGHT) {
        terminal_scroll(1);
        terminal_row = VGA_HEIGHT - 1;
    }
}

// Geriye sil (backspace)
void terminal_backspace(void) {
    if (terminal_column > 0) {
        terminal_column--;
        terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        terminal_move_cursor(terminal_column, terminal_row);
    } else if (terminal_row > 0) {
        terminal_row--;
        terminal_column = VGA_WIDTH - 1;
        terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        terminal_move_cursor(terminal_column, terminal_row);
    }
}

// Sil (delete)
void terminal_delete(void) {
    terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
    // İmleç hareket etmez
}

// Tab karakteri
void terminal_tab(void) {
    for (int i = 0; i < TAB_SIZE; i++) {
        terminal_putchar(' ');
    }
}

// Ekranı yukarıya kaydır
void terminal_scroll(int lines) {
    if (lines <= 0) return;
    
    // Üst n satırı kaldır ve diğer satırları yukarı taşı
    for (size_t y = lines; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t dst_index = (y - lines) * VGA_WIDTH + x;
            const size_t src_index = y * VGA_WIDTH + x;
            VGA_MEMORY[dst_index] = VGA_MEMORY[src_index];
        }
    }
    
    // Alt satırları temizle
    for (size_t y = VGA_HEIGHT - lines; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', terminal_color);
        }
    }
}

// Satır temizle
void terminal_clear_line(uint16_t line) {
    if (line >= VGA_HEIGHT) return;
    
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_putentryat(' ', terminal_color, x, line);
    }
}

// İmleçten sona kadar ekranı temizle
void terminal_clear_screen_from_cursor(void) {
    // İmleçten satır sonuna kadar temizle
    for (size_t x = terminal_column; x < VGA_WIDTH; x++) {
        terminal_putentryat(' ', terminal_color, x, terminal_row);
    }
    
    // Kalan satırları temizle
    for (size_t y = terminal_row + 1; y < VGA_HEIGHT; y++) {
        terminal_clear_line(y);
    }
}

// Başlangıçtan imlece kadar ekranı temizle
void terminal_clear_screen_to_cursor(void) {
    // İlk satırlardan terminal_row satırına kadar temizle
    for (size_t y = 0; y < terminal_row; y++) {
        terminal_clear_line(y);
    }
    
    // Terminal_row satırının başından imlece kadar temizle
    for (size_t x = 0; x <= terminal_column; x++) {
        terminal_putentryat(' ', terminal_color, x, terminal_row);
    }
}

// Karakter yazdır
void terminal_putchar(char c) {
    // Özel karakterleri işle
    if (c == '\n') {
        terminal_newline();
    } else if (c == '\r') {
        terminal_column = 0;
    } else if (c == '\b') {
        terminal_backspace();
    } else if (c == '\t') {
        terminal_tab();
    } else {
        // Normal karakter
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {
            terminal_newline();
        }
    }
    
    // İmleç konumunu güncelle
    terminal_move_cursor(terminal_column, terminal_row);
}

// String yaz
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

// String yaz (null sonlandırmalı)
void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

// Terminali temizle
void terminal_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', terminal_color);
        }
    }
    
    terminal_row = 0;
    terminal_column = 0;
    terminal_move_cursor(0, 0);
}

// İmleç konumunu ayarla
void terminal_move_cursor(uint16_t x, uint16_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    terminal_column = x;
    terminal_row = y;
    
    // Donanım imlecini güncelle
    uint16_t pos = y * VGA_WIDTH + x;
    
    // Yüksek ve düşük baytları ayır
    uint8_t low = (uint8_t)(pos & 0xFF);
    uint8_t high = (uint8_t)((pos >> 8) & 0xFF);
    
    // VGA portlarına yazarak imleci güncelle
    // 0x3D4 ve 0x3D5 portları CRT Controller'a komut/veri yazmak için kullanılır
    ASM_INLINE_INPUT2("outb %0, %1", "a"(14), "Nd"(0x3D4));
    ASM_INLINE_INPUT2("outb %0, %1", "a"(high), "Nd"(0x3D5));
    ASM_INLINE_INPUT2("outb %0, %1", "a"(15), "Nd"(0x3D4));
    ASM_INLINE_INPUT2("outb %0, %1", "a"(low), "Nd"(0x3D5));
}

// İmleç konumunu al
cursor_pos_t terminal_get_cursor(void) {
    cursor_pos_t pos;
    pos.x = terminal_column;
    pos.y = terminal_row;
    return pos;
}

// İmleci görünür/gizli yap
void terminal_set_cursor_visible(bool visible) {
    cursor_visible = visible;
    
    // İmleci gizle veya göster
    if (visible) {
        // Normal imleç (altçizgi biçimi)
        ASM_INLINE_INPUT2("outb %0, %1", "a"(10), "Nd"(0x3D4));
        ASM_INLINE_INPUT2("outb %0, %1", "a"(14), "Nd"(0x3D5));
        ASM_INLINE_INPUT2("outb %0, %1", "a"(11), "Nd"(0x3D4));
        ASM_INLINE_INPUT2("outb %0, %1", "a"(15), "Nd"(0x3D5));
    } else {
        // İmleci ekran dışına taşıyarak gizle
        ASM_INLINE_INPUT2("outb %0, %1", "a"(10), "Nd"(0x3D4));
        ASM_INLINE_INPUT2("outb %0, %1", "a"(0x1F), "Nd"(0x3D5));
        ASM_INLINE_INPUT2("outb %0, %1", "a"(11), "Nd"(0x3D4));
        ASM_INLINE_INPUT2("outb %0, %1", "a"(0x1F), "Nd"(0x3D5));
    }
}

// Tam sayıyı yazdır
void terminal_print_int(int value) {
    // Negatif sayılar için
    if (value < 0) {
        terminal_putchar('-');
        value = -value;
    }
    
    // Sayıyı basamaklarına ayır
    char buffer[12];  // 32-bit int için yeterli
    int i = 0;
    
    // Sayı 0 ise özel durum
    if (value == 0) {
        terminal_putchar('0');
        return;
    }
    
    // Rakamları ters sırada depola
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Ters sırada yazdır
    while (i > 0) {
        terminal_putchar(buffer[--i]);
    }
}

// İşaretsiz tam sayıyı yazdır
void terminal_print_uint(unsigned int value) {
    // Sayıyı basamaklarına ayır
    char buffer[12];  // 32-bit uint için yeterli
    int i = 0;
    
    // Sayı 0 ise özel durum
    if (value == 0) {
        terminal_putchar('0');
        return;
    }
    
    // Rakamları ters sırada depola
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Ters sırada yazdır
    while (i > 0) {
        terminal_putchar(buffer[--i]);
    }
}

// Onaltılık sayıyı yazdır
void terminal_print_hex(unsigned int value) {
    terminal_writestring("0x");
    
    // Sayı 0 ise özel durum
    if (value == 0) {
        terminal_putchar('0');
        return;
    }
    
    // Hex basamakları
    const char hex_digits[] = "0123456789ABCDEF";
    char buffer[9];  // 32-bit için yeterli + null terminator
    int i = 0;
    
    // Rakamları ters sırada depola
    while (value > 0 && i < 8) {
        buffer[i++] = hex_digits[value & 0xF];
        value >>= 4;
    }
    
    // Ters sırada yazdır
    while (i > 0) {
        terminal_putchar(buffer[--i]);
    }
}

// Ön plan rengini ayarla
void terminal_set_fg_color(enum vga_color fg) {
    uint8_t bg = terminal_color >> 4;
    terminal_color = vga_entry_color(fg, bg);
}

// Arka plan rengini ayarla
void terminal_set_bg_color(enum vga_color bg) {
    uint8_t fg = terminal_color & 0x0F;
    terminal_color = vga_entry_color(fg, bg);
}

// Rengi varsayılana sıfırla
void terminal_reset_color(void) {
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

// Terminal genişliğini al
uint16_t terminal_get_width(void) {
    return VGA_WIDTH;
}

// Terminal yüksekliğini al
uint16_t terminal_get_height(void) {
    return VGA_HEIGHT;
}

// Karakter al (bu şimdilik dummy bir fonksiyon, 
// keyboard.c ile entegre edilecek)
char terminal_getchar(void) {
    // Bu fonksiyon klavye sürücüsü tarafından 
    // implemente edilecek, şimdilik null dönüyoruz
    return KEY_NULL;
}

// String al
void terminal_gets(char* buffer, size_t size) {
    if (size == 0) return;
    
    size_t i = 0;
    char c;
    
    while (i < size - 1) {
        c = terminal_getchar();
        
        // Satır sonu
        if (c == KEY_ENTER) {
            buffer[i] = '\0';
            terminal_putchar('\n');
            break;
        }
        // Backspace
        else if (c == KEY_BACKSPACE && i > 0) {
            i--;
            terminal_backspace();
        }
        // Normal karakter
        else if (c >= ' ' && c <= '~') {
            buffer[i++] = c;
            terminal_putchar(c);
        }
    }
    
    // Hiçbir durum oluşmadıysa null ile sonlandır
    buffer[i] = '\0';
}

// Satır oku
int terminal_read_line(char* buffer, size_t size) {
    terminal_gets(buffer, size);
    return 0; // Başarı durumu
}

// Basit printf implementasyonu
void terminal_printf(const char* format, ...) {
    // stdio.h va_list, va_start ve va_arg olmadan
    // basit bir printf implementasyonu (ilerde genişletilecek)
    
    for (size_t i = 0; format[i] != '\0'; i++) {
        // Format belirteci değilse normal karakter yazdır
        if (format[i] != '%') {
            terminal_putchar(format[i]);
            continue;
        }
        
        // Format belirteci ise sonraki karaktere bak
        i++; // % işaretini atla
        
        // Format string biterse çık
        if (format[i] == '\0') break;
        
        // Format belirtecini işle
        switch (format[i]) {
            case '%':
                terminal_putchar('%');
                break;
            case 'd':
            case 'i':
                // Tam sayı yazdır (ilerde va_arg ile değiştirilecek)
                terminal_writestring("(int)");
                break;
            case 'u':
                // İşaretsiz tam sayı yazdır
                terminal_writestring("(uint)");
                break;
            case 'x':
            case 'X':
                // Hex yazdır
                terminal_writestring("(hex)");
                break;
            case 's':
                // String yazdır
                terminal_writestring("(string)");
                break;
            case 'c':
                // Karakter yazdır
                terminal_writestring("(char)");
                break;
            default:
                // Bilinmeyen format belirteci, olduğu gibi yazdır
                terminal_putchar('%');
                terminal_putchar(format[i]);
                break;
        }
    }
}
