#include "memory.h"
#include <kernel/types.h>
#include <drivers/terminal.h>

extern uint32_t* page_directory;

typedef unsigned int size_t;

#define __asm__ asm

typedef struct memory_block {
    void* address;
    size_t size;
    uint8_t used;
    struct memory_block* next;
} memory_block_t;
static memory_block_t* memory_blocks = NULL;

static inline void invlpg(void* addr) {
    // In real implementation:
    // __asm__ volatile("invlpg (%0)" : : "r" (addr) : "memory");
    
    (void)addr; // Use address but do nothing (to avoid warnings)
}

void* get_physaddr(void* virtualaddr) {
    uint32_t pdindex = (uint32_t)virtualaddr >> 22;
    uint32_t ptindex = (uint32_t)virtualaddr >> 12 & 0x03FF;
    
    if (!(page_directory[pdindex] & 1)) {
        return NULL;
    }
    
    uint32_t* pt = (uint32_t*)(page_directory[pdindex] & ~0xFFF);
    
    if (!(pt[ptindex] & 1)) {
        return NULL;
    }
    
    return (void*)((pt[ptindex] & ~0xFFF) + ((uint32_t)virtualaddr & 0xFFF));
}

void map_page(void* physaddr, void* virtualaddr, uint32_t flags) {
    uint32_t pdindex = (uint32_t)virtualaddr >> 22;
    uint32_t ptindex = (uint32_t)virtualaddr >> 12 & 0x03FF;
    
    uint32_t* pt = (uint32_t*)(page_directory[pdindex] & ~0xFFF);
    
    pt[ptindex] = ((uint32_t)physaddr) | (flags & 0xFFF) | 1;
    
    invlpg(virtualaddr);
}

void kfree(void* ptr) {
    if (!ptr) return;
    
    memory_block_t* block = memory_blocks;
    memory_block_t* prev = NULL;
    
    while (block != NULL) {
        if (block->address == ptr) {
            block->used = 0;
            
            for (size_t i = 0; i < block->size; i++) {
                ((uint8_t*)ptr)[i] = 0;
            }
            
            return;
        }
        
        prev = block;
        block = block->next;
    }
    
    terminal_writestring("HATA: Bilinmeyen bellek blogu serbest birakilmaya calisildi!\n");
}

void memory_check_leaks(void) {
    terminal_writestring("Bellek sizintisi kontrolu yapiliyor...\n");
    
    memory_block_t* block = memory_blocks;
    uint32_t leak_count = 0;
    size_t total_leaked = 0;
    
    while (block != NULL) {
        if (block->used) {
            leak_count++;
            total_leaked += block->size;
        }
        block = block->next;
    }
    
    if (leak_count > 0) {
        terminal_writestring("UYARI: Bellek sizintisi tespit edildi!\n");
    } else {
        terminal_writestring("Bellek sizintisi yok.\n");
    }
}
