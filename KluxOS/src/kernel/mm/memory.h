#ifndef MEMORY_H
#define MEMORY_H

#include <kernel/types.h>
#include <compat.h>  // Derleyici uyumluluğu için eklendi
#include <stddef.h>

typedef unsigned int size_t;

#define PAGE_SIZE 4096
#define PAGE_FAULT_PRESENT  0x1
#define PAGE_FAULT_WRITE    0x2
#define PAGE_FAULT_USER     0x4

#define MEMORY_REGION_AVAILABLE 1
#define MEMORY_REGION_RESERVED  2
#define MEMORY_REGION_ACPI      3
#define MEMORY_REGION_NVS       4
#define MEMORY_REGION_BADRAM    5

#define MEMORY_PRESENT    0x1
#define MEMORY_READWRITE  0x2
#define MEMORY_USER       0x4
#define MEMORY_WRITETHROUGH 0x8
#define MEMORY_NOCACHE    0x10
#define MEMORY_ACCESSED   0x20
#define MEMORY_DIRTY      0x40
#define MEMORY_FRAME      0xFFFFF000


typedef uint32_t phys_addr_t;
typedef uint32_t virt_addr_t;


typedef struct {
    uint64_t base_addr;   
    uint64_t length;      
    uint32_t type;        
} memory_region_t;


typedef struct {
    uint32_t total_memory;      
    uint32_t usable_memory;     
    uint32_t free_memory;       
    uint32_t used_memory;       
    uint32_t reserved_memory;   
    uint32_t num_regions;       
    memory_region_t* regions;   
} physical_memory_manager_t;


typedef struct {
    unsigned int present    : 1;   
    unsigned int rw         : 1;   
    unsigned int user       : 1;   
    unsigned int accessed   : 1;   
    unsigned int dirty      : 1;   
    unsigned int unused     : 7;   
    unsigned int frame      : 20;  
    
    unsigned int used       : 1;   
    unsigned int kernel_page : 1;  
    unsigned int ref_count  : 10;  
} page_frame_t;


typedef struct {
    uint32_t* tables[1024];          
    uint32_t tables_physical[1024];  
    uint32_t physical_addr;          
} page_directory_t;


void init_memory();  
void init_paging();  
void memory_info();  
void switch_page_directory(page_directory_t* dir);  
page_frame_t* get_page(uint32_t address, int make, page_directory_t* dir);  
void alloc_frame(page_frame_t* page, int is_kernel, int is_writeable);  
void free_frame(page_frame_t* page);  
void handle_page_fault(uint32_t error_code, uint32_t address);  
phys_addr_t get_physaddr(void* virtualaddr);  

void map_page(void* physaddr, void* virtualaddr, uint32_t flags);  
void unmap_page(void* virtualaddr);  

void* kmalloc(size_t size);  
void* kmalloc_aligned(size_t size);  
void* kmalloc_physical(size_t size, phys_addr_t* phys);  
void* kmalloc_aligned_physical(size_t size, phys_addr_t* phys);  
void kfree(void* ptr);  
void* krealloc(void* ptr, size_t size);  

void* memset(void* s, int c, size_t n);  
void* memcpy(void* dest, const void* src, size_t n);  

#if defined(__GNUC__) || defined(__clang__)
    extern void load_page_directory(uint32_t dir_addr) ALIGNED(4);
    extern void enable_paging() ALIGNED(4);
#endif

#endif // MEMORY_H 
