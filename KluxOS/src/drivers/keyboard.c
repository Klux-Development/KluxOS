#include <drivers/keyboard.h>
#include <drivers/terminal.h>
#include <kernel/types.h>
#include <kernel/interrupt.h>
#include <kernel/interrupt/idt.h>
#include <compat.h>  // Assembly uyumluluğu için eklendi

// Klavye tamponu
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static uint32_t buffer_start = 0;
static uint32_t buffer_end = 0;
static uint32_t buffer_count = 0;

// Klavye durumu
static key_state_t key_state = {
    .shift_pressed = false,
    .ctrl_pressed = false,
    .alt_pressed = false,
    .capslock_on = false,
    .numlock_on = false,
    .scrolllock_on = false
};

// Klavye modu
static keyboard_mode_t current_mode = KEYBOARD_MODE_ASCII;

// Klavye düzeni
static keyboard_layout_t current_layout = KEYBOARD_LAYOUT_US;

static const char us_keymap_lower[128] = {
    0, KEY_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', KEY_BACKSPACE,
    KEY_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', KEY_ENTER,
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char us_keymap_upper[128] = {
    0, KEY_ESCAPE, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', KEY_BACKSPACE,
    KEY_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', KEY_ENTER,
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char tr_keymap_lower[128] = {
    0, KEY_ESCAPE, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '*', '-', KEY_BACKSPACE,
    KEY_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'g', 'u', KEY_ENTER,
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 's', 'i', '"',
    0, ',', 'z', 'x', 'c', 'v', 'b', 'n', 'm', 'o', 'c', '.', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char tr_keymap_upper[128] = {
    0, KEY_ESCAPE, '!', '\'', '^', '+', '%', '&', '/', '(', ')', '=', '?', '_', KEY_BACKSPACE,
    KEY_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 'G', 'U', KEY_ENTER,
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'S', 'I', 'E',
    0, ';', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', 'O', 'C', ':', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Tampon işlemleri
static void buffer_push(char c) {
    if (buffer_count < KEYBOARD_BUFFER_SIZE) {
        keyboard_buffer[buffer_end] = c;
        buffer_end = (buffer_end + 1) % KEYBOARD_BUFFER_SIZE;
        buffer_count++;
    }
}

static char buffer_pop(void) {
    if (buffer_count == 0) {
        return 0;
    }
    
    char c = keyboard_buffer[buffer_start];
    buffer_start = (buffer_start + 1) % KEYBOARD_BUFFER_SIZE;
    buffer_count--;
    
    return c;
}

// PS/2 klavyeden veri oku
static uint8_t keyboard_read_scancode(void) {
    return inb(KEYBOARD_DATA_PORT);
}

// Tuş kodu -> ASCII dönüşümü
static char keycode_to_ascii(uint8_t keycode) {
    // Basım/bırakış ayrımı
    bool is_key_release = (keycode & 0x80) != 0;
    uint8_t key = keycode & 0x7F; // 7-bit tuş kodu
    
    // Tuş bırakışını işle
    if (is_key_release) {
        // Shift, Ctrl, Alt bırakışını kontrol et
        switch (key) {
            case KEYCODE_LSHIFT:
            case KEYCODE_RSHIFT:
                key_state.shift_pressed = false;
                break;
            case KEYCODE_LCTRL:
                key_state.ctrl_pressed = false;
                break;
            case KEYCODE_LALT:
                key_state.alt_pressed = false;
                break;
        }
        return 0; // Tuş bırakışları karakter üretmez
    }
    
    // Tuş basışlarını işle
    switch (key) {
        case KEYCODE_LSHIFT:
        case KEYCODE_RSHIFT:
            key_state.shift_pressed = true;
            return 0;
        case KEYCODE_LCTRL:
            key_state.ctrl_pressed = true;
            return 0;
        case KEYCODE_LALT:
            key_state.alt_pressed = true;
            return 0;
        case KEYCODE_CAPSLOCK:
            key_state.capslock_on = !key_state.capslock_on;
            return 0;
        case KEYCODE_NUMLOCK:
            key_state.numlock_on = !key_state.numlock_on;
            return 0;
        case KEYCODE_SCROLLLOCK:
            key_state.scrolllock_on = !key_state.scrolllock_on;
            return 0;
    }
    
    // Özel tuşları işle
    switch (key) {
        case KEYCODE_ENTER:
            return KEY_ENTER;
        case KEYCODE_BACKSPACE:
            return KEY_BACKSPACE;
        case KEYCODE_TAB:
            return KEY_TAB;
        case KEYCODE_ESCAPE:
            return KEY_ESCAPE;
        case KEYCODE_HOME:
            return KEY_HOME;
        case KEYCODE_END:
            return KEY_END;
        case KEYCODE_INSERT:
            return KEY_NULL; // Şu anda desteklenmiyor
        case KEYCODE_DELETE:
            return KEY_DELETE;
        case KEYCODE_PGUP:
            return KEY_PAGE_UP;
        case KEYCODE_PGDN:
            return KEY_PAGE_DOWN;
        case KEYCODE_LEFT:
            return KEY_ARROW_LEFT;
        case KEYCODE_RIGHT:
            return KEY_ARROW_RIGHT;
        case KEYCODE_UP:
            return KEY_ARROW_UP;
        case KEYCODE_DOWN:
            return KEY_ARROW_DOWN;
    }
    
    // Normal karakterleri işle
    bool use_uppercase = (key_state.shift_pressed != key_state.capslock_on);
    
    if (current_layout == KEYBOARD_LAYOUT_US) {
        return use_uppercase ? us_keymap_upper[key] : us_keymap_lower[key];
    } else { // KEYBOARD_LAYOUT_TR
        return use_uppercase ? tr_keymap_upper[key] : tr_keymap_lower[key];
    }
}

// Klavye kesme işleyicisi
void keyboard_handler(void) {
    // Kesmeyi onayla
    ack_irq(1); // IRQ1 - klavye kesmesi
    
    // Tuş kodu oku
    uint8_t scancode = keyboard_read_scancode();
    
    // Çalışma moduna göre işle
    if (current_mode == KEYBOARD_MODE_RAW) {
        // Ham tuş kodu için
        buffer_push(scancode);
    } else {
        // ASCII modları için
        char ascii = keycode_to_ascii(scancode);
        
        // Eğer geçerli bir karakter ise tampona ekle
        if (ascii != 0) {
            buffer_push(ascii);
            
            // ECHO modunda ekrana yazdır
            if (current_mode == KEYBOARD_MODE_ECHO) {
                terminal_putchar(ascii);
            }
        }
    }
}

// Klavyeyi başlat
void keyboard_init(void) {
    register_interrupt_handler(IRQ1, keyboard_handler);
    
    key_state.capslock_on = false;
    key_state.numlock_on = false;
    key_state.scrolllock_on = false;
    
    buffer_start = 0;
    buffer_end = 0;
    buffer_count = 0;
    
    current_mode = KEYBOARD_MODE_ASCII;
    current_layout = KEYBOARD_LAYOUT_US;
    
    terminal_writestring("Keyboard driver initialized.\n");
}

// Klavye modunu ayarla
void keyboard_set_mode(keyboard_mode_t mode) {
    current_mode = mode;
}

// Klavye düzenini ayarla
void keyboard_set_layout(keyboard_layout_t layout) {
    current_layout = layout;
}

// Tampon durumu
bool keyboard_data_available(void) {
    return buffer_count > 0;
}

// Tampondan karakter al (bloke edici değil)
char keyboard_read(void) {
    if (buffer_count > 0) {
        return buffer_pop();
    }
    return 0;
}

// Karakter oku (bloke edici)
char keyboard_getchar(void) {
    char c;
    
    // Tampon dolana kadar bekle
    while (!keyboard_data_available()) {
        ASM_INLINE("hlt");
    }
    
    c = buffer_pop();
    return c;
}

// Terminal okuma fonksiyonlarını bağla
void keyboard_connect_terminal(void) {
    // Terminal'in getchar fonksiyonunu bizim getchar'a bağla
    // Bu fonksiyon terminal_getchar fonksiyonunu bizim getchar'a yönlendirir
    // Not: Bu sadece bir örnek, gerçek implementasyon farklı olabilir
    
    // terminal_getchar = keyboard_getchar;
}

// Port okuma işlevi (Assembly olmadan)
uint8_t inb(uint16_t port) {
    uint8_t ret;
    ASM_INLINE_PARAMS("inb %1, %0", "=a"(ret), "Nd"(port));
    return ret;
}


void outb(uint16_t port, uint8_t val) {
    ASM_INLINE_INPUT2("outb %0, %1", "a"(val), "Nd"(port));
}

static void ack_irq(uint8_t irq) {

    outb(0x20, 0x20); // Master PIC EOI (End of Interrupt)
    
    if (irq > 7) {
        outb(0xA0, 0x20); // Slave PIC EOI
    }
}
