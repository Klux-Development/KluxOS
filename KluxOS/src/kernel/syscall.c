#include <kernel/syscall.h>
#include <kernel/types.h>
#include <kernel/interrupt.h>
#include <kernel/process.h>
#include <kernel/fs.h>
#include <drivers/terminal.h>
#include "mm/memory.h"

#define MAX_FD 32


typedef struct {
    bool used;           
    fs_node_t* node;     
    uint32_t offset;     
    uint32_t flags;      
} fd_entry_t;

static fd_entry_t fd_table[MAX_FD];

static void* syscall_table[32];

void init_syscalls(void) {
    terminal_writestring("System calls are being initialized...\n");
    
    syscall_table[SYS_EXIT] = syscall_exit;
    syscall_table[SYS_FORK] = syscall_fork;
    syscall_table[SYS_READ] = syscall_read;
    syscall_table[SYS_WRITE] = syscall_write;
    syscall_table[SYS_OPEN] = syscall_open;
    syscall_table[SYS_CLOSE] = syscall_close;
    syscall_table[SYS_EXEC] = syscall_exec;
    syscall_table[SYS_TIME] = syscall_time;
    syscall_table[SYS_GETPID] = syscall_getpid;
    syscall_table[SYS_SLEEP] = syscall_sleep;
    syscall_table[SYS_MALLOC] = syscall_malloc;
    syscall_table[SYS_FREE] = syscall_free;
    
    register_interrupt_handler(0x80, (isr_t)syscall_handler);
    
    terminal_writestring("System calls are initialized.\n");
}

// System call handler (Interrupt 0x80)
void syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    if (syscall_num >= 32 || syscall_table[syscall_num] == 0) {
        terminal_writestring("ERROR: Invalid system call!\n");
        return;
    }
    
    void (*func)(uint32_t, uint32_t, uint32_t) = syscall_table[syscall_num];
    func(arg1, arg2, arg3);
}

void syscall_exit(int status) {
    process_t* current = process_get_current();
    if (current) {
        terminal_writestring("System call: exit() - Process is being terminated.\n");
        process_terminate(current);
    }
}

int syscall_fork(void) {
    terminal_writestring("System call: fork() - New process is being created.\n");
    process_t* current = process_get_current();
    
    return -1;
}

size_t syscall_read(int fd, void* buf, size_t count) {
    if (fd < 0 || fd >= MAX_FD || !fd_table[fd].used) {
        terminal_writestring("ERROR: Invalid file descriptor\n");
        return -1;
    }
    
    // stdin operation (fd=0)
    if (fd == 0) {
        extern char keyboard_buffer[256];
        extern int keyboard_buffer_pos;
        
        size_t available = keyboard_buffer_pos;
        size_t to_read = count > available ? available : count;
        
        if (to_read > 0) {
            memcpy(buf, keyboard_buffer, to_read);
            
            if (to_read < keyboard_buffer_pos) {
                for (size_t i = 0; i < keyboard_buffer_pos - to_read; i++) {
                    keyboard_buffer[i] = keyboard_buffer[i + to_read];
                }
                keyboard_buffer_pos -= to_read;
            } else {
                keyboard_buffer_pos = 0;
            }
            
            return to_read;
        }
        
        return 0;
    }
    
    fs_node_t* node = fd_table[fd].node;
    if (!node) {
        terminal_writestring("ERROR: Invalid file descriptor\n");
        return -1;
    }
    
    if (!(node->mask & FS_PERM_READ)) {
        terminal_writestring("ERROR: Read permission denied\n");
        return -1;
    }
    
    uint32_t read_size = 0;
    if (node->read) {
        read_size = node->read(node, fd_table[fd].offset, count, (uint8_t*)buf);
    } else {
        if (node->contents) {
            uint32_t remaining = node->length - fd_table[fd].offset;
            read_size = count > remaining ? remaining : count;
            
            if (read_size > 0) {
                memcpy(buf, (uint8_t*)node->contents + fd_table[fd].offset, read_size);
            }
        }
    }
    

    fd_table[fd].offset += read_size;
    
    return read_size;
}


size_t syscall_write(int fd, const void* buf, size_t count) {
    if (fd < 0 || fd >= MAX_FD) {
        terminal_writestring("ERROR: Invalid file descriptor\n");
        return -1;
    }
    
    // stdout/stderr operation (fd=1, fd=2)
    if (fd == 1 || fd == 2) {
        const char* str = (const char*)buf;
        for (size_t i = 0; i < count; i++) {
            terminal_putchar(str[i]);
        }
        return count;
    }
    
    if (!fd_table[fd].used) {
        terminal_writestring("ERROR: Unused file descriptor\n");
        return -1;
    }
    
    fs_node_t* node = fd_table[fd].node;
    if (!node) {
        terminal_writestring("ERROR: Invalid file descriptor\n");
        return -1;
    }
    

    if (!(node->mask & FS_PERM_WRITE)) {
        terminal_writestring("ERROR: Write permission denied\n");
        return -1;
    }
    

    uint32_t write_size = 0;
    if (node->write) {
        write_size = node->write(node, fd_table[fd].offset, count, (uint8_t*)buf);
    } else {

        uint32_t new_size = fd_table[fd].offset + count;
        

        if (new_size > node->length) {
            uint8_t* new_contents = (uint8_t*)krealloc(node->contents, new_size);
            if (!new_contents) {
                terminal_writestring("ERROR: File could not be resized - insufficient memory\n");
                return -1;
            }
            node->contents = new_contents;
            

            if (new_size > node->length) {
                memset((uint8_t*)node->contents + node->length, 0, new_size - node->length);
            }
            
            node->length = new_size;
        }
        

        if (count > 0) {
            memcpy((uint8_t*)node->contents + fd_table[fd].offset, buf, count);
            write_size = count;
        }
    }
    

    fd_table[fd].offset += write_size;
    

    extern uint32_t get_tick_count(void);
    node->modified_time = get_tick_count();
    
    return write_size;
}


static int find_free_fd(void) {
    for (int i = 3; i < MAX_FD; i++) {  
        if (!fd_table[i].used) {
            return i;
        }
    }
    return -1;  
}


#define O_RDONLY  0x0001
#define O_WRONLY  0x0002
#define O_RDWR    0x0003
#define O_CREAT   0x0100
#define O_TRUNC   0x0200
#define O_APPEND  0x0400


int syscall_open(const char* pathname, int flags) {
    if (!pathname) {
        return -1;
    }
    
    fs_node_t* node = fs_resolve_path(pathname);
    
    if (!node && !(flags & O_CREAT)) {
        terminal_writestring("ERROR: File not found: ");
        terminal_writestring(pathname);
        terminal_writestring("\n");
        return -1;
    }
    
    if (!node && (flags & O_CREAT)) {
        fs_node_t* parent = fs_get_parent_dir(pathname);
        if (!parent) {
            terminal_writestring("ERROR: Parent directory not found: ");
            terminal_writestring(pathname);
            terminal_writestring("\n");
            return -1;
        }
        
        const char* last_slash = strrchr(pathname, '/');
        const char* filename = last_slash ? last_slash + 1 : pathname;
        
        node = fs_create_node((char*)filename, FS_FILE);
        if (!node) {
            terminal_writestring("ERROR: Could not create file: ");
            terminal_writestring(pathname);
            terminal_writestring("\n");
            return -1;
        }
        
        fs_add_node(parent, node);
        terminal_writestring("File created: ");
        terminal_writestring(pathname);
        terminal_writestring("\n");
    }
    
    if (node->type != FS_FILE) {
        terminal_writestring("ERROR: Attempted to open a directory: ");
        terminal_writestring(pathname);
        terminal_writestring("\n");
        return -1;
    }
    
    if (flags & O_TRUNC) {
        if (node->truncate) {
            node->truncate(node);
        } else {
            if (node->length > 0 && node->contents) {
                memset(node->contents, 0, node->length);
                node->length = 0;
            }
        }
    }
    
    int fd = find_free_fd();
    if (fd < 0) {
        terminal_writestring("ERROR: Maximum number of open files reached\n");
        return -1;
    }
    
    fd_table[fd].node = node;
    fd_table[fd].offset = (flags & O_APPEND) ? node->length : 0;
    fd_table[fd].used = 1;
    
    if (node->open) {
        node->open(node);
    }
    
    return fd;
}


int syscall_close(int fd) {
    if (fd < 0 || fd >= MAX_FD) {
        terminal_writestring("ERROR: Invalid file descriptor\n");
        return -1;
    }
    
    if (fd <= 2) {
        terminal_writestring("ERROR: Cannot close standard I/O descriptors\n");
        return -1;
    }
    
    if (!fd_table[fd].used) {
        terminal_writestring("ERROR: Descriptor is already closed\n");
        return -1;
    }
    
    fs_node_t* node = fd_table[fd].node;
    if (node && node->close) {
        node->close(node);
    }
    
    fd_table[fd].node = NULL;
    fd_table[fd].offset = 0;
    fd_table[fd].used = 0;
    
    return 0;
}


int syscall_exec(const char* path, char* const argv[]) {
    terminal_writestring("System call: exec()\n");
    return -1;
}


uint32_t syscall_time(void) {
    terminal_writestring("System call: time()\n");
    return 0;
}


uint32_t syscall_getpid(void) {
    process_t* current = process_get_current();
    if (current) {
        return current->pid;
    }
    return 0;
}


void syscall_sleep(uint32_t ms) {
    terminal_writestring("System call: sleep()\n");
    // TODO: Implement sleep
}


void* syscall_malloc(size_t size) {
    terminal_writestring("System call: malloc()\n");
    return NULL;
}


void syscall_free(void* ptr) {
    terminal_writestring("System call: free()\n");
}
