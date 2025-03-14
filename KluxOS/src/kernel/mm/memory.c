#include "memory.h"
#include <kernel/types.h>
#include <drivers/terminal.h>

extern void* multiboot_info;

#if defined(__GNUC__) || defined(__clang__)
    #define ASM_INLINE(x) __asm__ volatile(x)
    #define HAVE_INLINE_ASM 1
#elif defined(_MSC_VER)
    // MSVC için inline assembly
    #define ASM_INLINE(x) __asm x
    #define HAVE_INLINE_ASM 1
#else
    // Diğer derleyiciler için boş makro
    #define ASM_INLINE(x) /* assembly devre dışı */
    #define HAVE_INLINE_ASM 0
#endif

#define asm __asm__
#define __asm__ asm

static physical_memory_manager_t phys_mem_mgr;
static page_directory_t* kernel_directory = NULL;
static page_directory_t* current_directory = NULL;

static memory_region_t memory_regions[64];
static uint32_t memory_region_count = 0;


#define MAX_PAGES 1048576   // 4 GB / 4 KB = 1048576 page
static uint32_t frames[MAX_PAGES / 32];
static uint32_t nframes = 0;

static uint32_t get_frame_index(phys_addr_t addr) {
    return addr / PAGE_SIZE;
}


static void set_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    frames[idx] |= (1 << off);
}


static void clear_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    frames[idx] &= ~(1 << off);
}

// Bit haritasında belirtilen sayfanın durumunu kontrol et
static uint32_t test_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    return (frames[idx] & (1 << off));
}

// İlk boş sayfayı bul
static uint32_t first_free_frame() {
    for (uint32_t i = 0; i < nframes / 32; i++) {
        if (frames[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                uint32_t test = 1 << j;
                if (!(frames[i] & test)) {
                    return i * 32 + j;
                }
            }
        }
    }
    return 0;
}


void alloc_frame(page_frame_t* frame, int is_kernel, int is_writeable) {
    if (frame->used) {
        return; 
    }
    
    uint32_t idx = first_free_frame();
    if (idx == 0xFFFFFFFF) {
        terminal_writestring("HATA: Yeterli bellek yok!\n");
        for(;;);
    }
    
    set_frame(idx * PAGE_SIZE);
    frame->used = 1;
    frame->kernel_page = is_kernel ? 1 : 0;
    frame->ref_count = 1;
    
    uint32_t flags = MEMORY_PRESENT;
    if (is_writeable) flags |= MEMORY_READWRITE;
    if (!is_kernel) flags |= MEMORY_USER;
    
 
    phys_addr_t physaddr = idx * PAGE_SIZE;
    map_page((void*)physaddr, (void*)frame, flags);
}


void free_frame(page_frame_t* frame) {
    if (!frame->used) {
        return; 
    }
    
    phys_addr_t physaddr = (phys_addr_t)get_physaddr((void*)frame);
    clear_frame(physaddr);
    frame->used = 0;
    frame->ref_count = 0;
}


void init_memory() {
    terminal_writestring("Fiziksel bellek yoneticisi baslatiliyor...\n");
    
    heap_end = (uint32_t)&kernel_heap[0]; 
    
   
    phys_mem_mgr.total_memory = 128 * 1024; // 128 MB total memory
    phys_mem_mgr.usable_memory = 96 * 1024; // 96 MB usable memory
    phys_mem_mgr.free_memory = 96 * 1024;   // At start, all usable memory is free
    phys_mem_mgr.used_memory = 0;           // At start, no memory is used
    phys_mem_mgr.reserved_memory = 32 * 1024; // 32 MB reserved memory (BIOS, hardware, etc.)
    phys_mem_mgr.num_regions = 2;           // Two regions: available and reserved
    phys_mem_mgr.regions = memory_regions;
    
    memory_regions[0].base_addr = 0x100000;    // Start at 1 MB
    memory_regions[0].length = 96 * 1024 * 1024; // 96 MB usable
    memory_regions[0].type = MEMORY_REGION_AVAILABLE;
    
    memory_regions[1].base_addr = 96 * 1024 * 1024 + 0x100000;
    memory_regions[1].length = 32 * 1024 * 1024;
    memory_regions[1].type = MEMORY_REGION_RESERVED;
    
    terminal_writestring("  Toplam Bellek: ");
    terminal_print_int(phys_mem_mgr.total_memory);
    terminal_writestring(" KB\n");
    
    terminal_writestring("  Kullanilabilir Bellek: ");
    terminal_print_int(phys_mem_mgr.usable_memory);
    terminal_writestring(" KB\n");
    
    terminal_writestring("  Ayrilmis Bellek: ");
    terminal_print_int(phys_mem_mgr.reserved_memory);
    terminal_writestring(" KB\n\n");
    
    nframes = phys_mem_mgr.total_memory * 1024 / PAGE_SIZE;
    for (uint32_t i = 0; i < nframes / 32; i++) {
        frames[i] = 0;
    }
    
    for (uint32_t i = 0; i < 256; i++) {
        set_frame(i * PAGE_SIZE);
    }
    
    init_paging();
    
    terminal_writestring("Bellek yonetimi baslatildi.\n");
}

void init_paging() {
    terminal_writestring("Sayfalama sistemi baslatiliyor...\n");
    
    kernel_directory = (page_directory_t*)kmalloc_aligned(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));
    current_directory = kernel_directory;
    
    for (uint32_t i = 0; i < 1024; i++) {
        page_frame_t* page = get_page(i * PAGE_SIZE, 1, kernel_directory);
        alloc_frame(page, 1, 0); // Çekirdek, sadece okuma
    }
    
    // page error handler (must be added to interrupt.c)
    // register_interrupt_handler(14, handle_page_fault);
    
    // load page directory to CR3
    switch_page_directory(kernel_directory);
    
    terminal_writestring("Sayfalama sistemi baslatildi.\n");
}

void switch_page_directory(page_directory_t* dir) {
    current_directory = dir;
    
#if HAVE_INLINE_ASM
    ASM_INLINE("mov %0, %%cr3" : : "r"(dir->physical_addr));
    
    uint32_t cr0;
    ASM_INLINE("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // PG bayrağını etkinleştir
    ASM_INLINE("mov %0, %%cr0" : : "r"(cr0));
#elif defined(__GNUC__) || defined(__clang__)
    load_page_directory(dir->physical_addr);
    enable_paging();
#else
    terminal_writestring("UYARI: Assembly desteği olmadan sayfalama aktifleştirilemez!\n");
#endif
}

page_frame_t* get_page(uint32_t address, int make, page_directory_t* dir) {
    address /= PAGE_SIZE;
    uint32_t table_idx = address / 1024;
    
    if (make && !dir->tables[table_idx]) {

        uint32_t phys;
        dir->tables[table_idx] = (uint32_t*)kmalloc_aligned_physical(sizeof(uint32_t) * 1024, &phys);
        dir->tables_physical[table_idx] = phys | MEMORY_PRESENT | MEMORY_READWRITE | MEMORY_USER;
        
        memset(dir->tables[table_idx], 0, sizeof(uint32_t) * 1024);
        return &dir->tables[table_idx][address % 1024];
    }
    
    return dir->tables[table_idx] ? &dir->tables[table_idx][address % 1024] : NULL;
}

void handle_page_fault(uint32_t error_code, uint32_t address) {
    terminal_set_fg_color(VGA_COLOR_RED);
    terminal_writestring("SAYFA HATASI: 0x");
    terminal_print_hex(address);
    terminal_writestring(" (");
    
    if (!(error_code & PAGE_FAULT_PRESENT)) {
        terminal_writestring("sayfa mevcut degil");
    } else {
        terminal_writestring("sayfa koruma ihlali");
    }
    
    if (error_code & PAGE_FAULT_WRITE) {
        terminal_writestring(", yazma");
    } else {
        terminal_writestring(", okuma");
    }
    
    if (error_code & PAGE_FAULT_USER) {
        terminal_writestring(", kullanici modu");
    } else {
        terminal_writestring(", cekirdek modu");
    }
    
    terminal_writestring(")\n");
    terminal_reset_color();
    
    for(;;);
}

// ========= memory allocation functions =========

#define HEAP_SIZE 0x100000
static uint8_t kernel_heap[HEAP_SIZE];
static uint32_t heap_end = 0;  // start value

static uint32_t kmalloc_internal(size_t size, int align, phys_addr_t* phys) {
    // if aligned memory is requested
    if (align && (heap_end & 0xFFF)) {
        heap_end = (heap_end + 0x1000) & 0xFFFFF000;
    }
    
    // save current heap_end
    uint32_t addr = heap_end;
    
    if (phys) {
        *phys = (phys_addr_t)get_physaddr((void*)addr);
    }
    
    heap_end += size;
    
    return addr;
}

void* kmalloc(size_t size) {
    return (void*)kmalloc_internal(size, 0, NULL);
}

void* kmalloc_aligned(size_t size) {
    return (void*)kmalloc_internal(size, 1, NULL);
}

void* kmalloc_physical(size_t size, phys_addr_t* phys) {
    return (void*)kmalloc_internal(size, 0, phys);
}

void* kmalloc_aligned_physical(size_t size, phys_addr_t* phys) {
    return (void*)kmalloc_internal(size, 1, phys);
}

// Belleği belirli bir değerle doldur
void* memset(void* s, int c, size_t n) {
    unsigned char* p = (unsigned char*)s;
    
    for (size_t i = 0; i < n; i++) {
        p[i] = (unsigned char)c;
    }
    
    return s;
}

// Bellek kopyalama fonksiyonu
void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

void memory_info() {
    terminal_set_fg_color(VGA_COLOR_LIGHT_CYAN);
    terminal_writestring("\nBellek Bilgileri:\n");
    terminal_writestring("----------------\n");
    terminal_reset_color();
    
    terminal_writestring("Toplam Bellek: ");
    terminal_print_int(phys_mem_mgr.total_memory);
    terminal_writestring(" KB\n");
    
    terminal_writestring("Kullanilabilir Bellek: ");
    terminal_print_int(phys_mem_mgr.usable_memory);
    terminal_writestring(" KB\n");
    
    terminal_writestring("Bos Bellek: ");
    terminal_print_int(phys_mem_mgr.free_memory);
    terminal_writestring(" KB\n");
    
    terminal_writestring("Kullanilan Bellek: ");
    terminal_print_int(phys_mem_mgr.used_memory);
    terminal_writestring(" KB\n");
    
    terminal_writestring("Ayrilmis Bellek: ");
    terminal_print_int(phys_mem_mgr.reserved_memory);
    terminal_writestring(" KB\n");
    
    terminal_writestring("Cekirdek Yigin Durumu: ");
    terminal_print_int(heap_end - (uint32_t)kernel_heap);
    terminal_writestring(" / ");
    terminal_print_int(HEAP_SIZE);
    terminal_writestring(" byte kullaniliyor\n");
    
    terminal_writestring("\n");
}

// Belleği serbest bırak
void kfree(void* ptr) {
    if (ptr == NULL) return;
    
    // Burada gerçek bir bellek yöneticisi olmalı
    // Şu an için basit bir stub fonksiyonu
    
    // İleriki implementasyonda, heap yöneticisi burada çağrılacak
    terminal_writestring("kfree: Bellek serbest bırakıldı (stub)\n");
}

// Belleği yeniden boyutlandır
void* krealloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return kmalloc(size);
    }
    
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    // Yeni bellek bloğu ayır
    void* new_ptr = kmalloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }
    
    // Eski verileri yeni bloğa kopyala
    // NOT: Gerçek implementasyonda eski blok boyutunu bilmemiz gerekir
    // Şu an için sabit 256 byte varsayıyoruz
    size_t copy_size = size < 256 ? size : 256;
    memcpy(new_ptr, ptr, copy_size);
    
    // Eski belleği serbest bırak
    kfree(ptr);
    
    return new_ptr;
} 
