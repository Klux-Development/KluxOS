#ifndef SYSCALL_H
#define SYSCALL_H

#include <kernel/types.h>

// Sistem çağrı numaraları
enum {
    SYS_EXIT = 1,
    SYS_FORK = 2,
    SYS_READ = 3,
    SYS_WRITE = 4,
    SYS_OPEN = 5,
    SYS_CLOSE = 6,
    SYS_EXEC = 7,
    SYS_TIME = 8,
    SYS_GETPID = 9,
    SYS_SLEEP = 10,
    SYS_MALLOC = 11,
    SYS_FREE = 12
};

// Sistem çağrı işleyicisi
void syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

// Sistem çağrı fonksiyonları
void syscall_exit(int status);
int syscall_fork(void);
size_t syscall_read(int fd, void* buf, size_t count);
size_t syscall_write(int fd, const void* buf, size_t count);
int syscall_open(const char* pathname, int flags);
int syscall_close(int fd);
int syscall_exec(const char* path, char* const argv[]);
uint32_t syscall_time(void);
uint32_t syscall_getpid(void);
void syscall_sleep(uint32_t ms);
void* syscall_malloc(size_t size);
void syscall_free(void* ptr);

// Sistem çağrıları başlatma
void init_syscalls(void);

#endif
