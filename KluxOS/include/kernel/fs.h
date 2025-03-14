#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <kernel/types.h>

typedef enum {
    FS_FILE,
    FS_DIRECTORY,
    FS_CHARDEVICE,
    FS_BLOCKDEVICE,
    FS_PIPE,
    FS_SYMLINK
} fs_node_type_t;

typedef enum {
    FS_PERM_READ = 1,
    FS_PERM_WRITE = 2,
    FS_PERM_EXEC = 4
} fs_permission_t;

struct fs_node;

typedef uint32_t (*read_type_t)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*write_type_t)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef struct fs_node* (*readdir_type_t)(struct fs_node*, uint32_t);
typedef struct fs_node* (*finddir_type_t)(struct fs_node*, char* name);

typedef struct fs_node* (*readlink_type_t)(struct fs_node*);

typedef void (*truncate_type_t)(struct fs_node*);

typedef struct fs_node {
    char name[128];               
    uint32_t mask;                
    uint32_t uid;                 
    uint32_t gid;                 
    uint32_t flags;               
    uint32_t inode;               
    uint32_t length;              
    uint32_t created_time;        
    uint32_t modified_time;       
    uint32_t accessed_time;       
    fs_node_type_t type;          
    
    void* contents;               
    
    read_type_t read;             
    write_type_t write;           
    open_type_t open;             
    close_type_t close;           
    readdir_type_t readdir;       
    finddir_type_t finddir;       
    readlink_type_t readlink;     
    truncate_type_t truncate;     
    
    struct fs_node* parent;       
    struct fs_node* next;         
    struct fs_node* children;     
    struct fs_node* symlink_target;
    void* device;                
} fs_node_t;

extern fs_node_t* fs_root;

void fs_init(void);

uint32_t fs_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t fs_write(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
void fs_open(fs_node_t* node);
void fs_close(fs_node_t* node);
fs_node_t* fs_readdir(fs_node_t* node, uint32_t index);
fs_node_t* fs_finddir(fs_node_t* node, char* name);

fs_node_t* fs_create_node(char* name, fs_node_type_t type);
void fs_add_node(fs_node_t* parent, fs_node_t* child);
int fs_remove_node(fs_node_t* parent, fs_node_t* node);

fs_node_t* fs_create_symlink(char* name, fs_node_t* target);
fs_node_t* fs_readlink(fs_node_t* link);

fs_node_t* fs_resolve_path(const char* path);
fs_node_t* fs_get_parent_dir(const char* path);

int fs_check_permission(fs_node_t* node, uint32_t access_mask, uint32_t uid, uint32_t gid);

int strcmp(const char* s1, const char* s2);
char* strrchr(const char* s, int c);

void fs_test(void);

#endif
