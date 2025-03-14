#include <kernel/fs.h>
#include <kernel/types.h>
#include <drivers/terminal.h>
#include "mm/memory.h"
#include <libc/string.h>


fs_node_t* fs_root = NULL;

void fs_init(void) {
    terminal_writestring("Dosya sistemi baslatiliyor...\n");
    
    fs_root = fs_create_node("/", FS_DIRECTORY);
    if (fs_root) {
        fs_root->mask = FS_PERM_READ | FS_PERM_WRITE;
        fs_root->uid = 0;
        fs_root->gid = 0;
        
        terminal_writestring("Kok dizin olusturuldu.\n");
    } else {
        terminal_writestring("HATA: Kok dizin olusturulamadi!\n");
    }
    
    terminal_writestring("Dosya sistemi baslatildi.\n");
}

fs_node_t* fs_create_node(char* name, fs_node_type_t type) {
    fs_node_t* node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    if (!node) return NULL;
    
    memset(node, 0, sizeof(fs_node_t));
    
    int i;
    for (i = 0; name[i] != '\0' && i < 127; i++) {
        node->name[i] = name[i];
    }
    node->name[i] = '\0';
    
    extern uint32_t get_tick_count(void);
    node->created_time = get_tick_count();
    node->modified_time = node->created_time;
    
    node->type = type;
    node->length = 0;
    node->mask = FS_PERM_READ;
    

    if (type == FS_DIRECTORY) {
        node->readdir = fs_readdir;
        node->finddir = fs_finddir;
    }
    
    if (type == FS_FILE) {
        node->read = fs_read;
        node->write = fs_write;
    }
    
    node->open = fs_open;
    node->close = fs_close;
    
    return node;
}

void fs_add_node(fs_node_t* parent, fs_node_t* child) {
    if (!parent || !child) return;
    
    if (parent->type != FS_DIRECTORY) {
        terminal_writestring("HATA: Dizin olmayan bir dugume alt dugum eklenemez!\n");
        return;
    }
    
    child->parent = parent;
    
    if (parent->children == NULL) {
        parent->children = child;
    } else {
        fs_node_t* current = parent->children;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = child;
    }
}

int fs_remove_node(fs_node_t* parent, fs_node_t* node) {
    if (!parent || !node) return -1;
    
    if (parent->type != FS_DIRECTORY) {
        terminal_writestring("HATA: Dizin olmayan bir dugumden alt dugum kaldirilamaz!\n");
        return -1;
    }
    
    // Çocuk düğümü ebeveynden ayır
    if (parent->children == node) {
        parent->children = node->next;
    } else {
        fs_node_t* current = parent->children;
        while (current != NULL && current->next != node) {
            current = current->next;
        }
        
        if (current != NULL) {
            current->next = node->next;
        } else {
            return -1;
        }
    }
    
    node->parent = NULL;
    node->next = NULL;
    
    return 0;
}

uint32_t fs_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node && node->read) {
        return node->read(node, offset, size, buffer);
    }
    return 0;
}

uint32_t fs_write(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node && node->write) {
        return node->write(node, offset, size, buffer);
    }
    return 0;
}

void fs_open(fs_node_t* node) {
    if (node && node->open) {
        node->open(node);
    }
}

void fs_close(fs_node_t* node) {
    if (node && node->close) {
        node->close(node);
    }
}

fs_node_t* fs_readdir(fs_node_t* node, uint32_t index) {
    if (node && node->type == FS_DIRECTORY && node->readdir) {
        return node->readdir(node, index);
    }
    return NULL;
}

fs_node_t* fs_finddir(fs_node_t* node, char* name) {
    if (node && node->type == FS_DIRECTORY && node->finddir) {
        return node->finddir(node, name);
    }
    return NULL;
}

int fs_check_permission(fs_node_t* node, uint32_t access_mask, uint32_t uid, uint32_t gid) {
    if (!node) return 0;
    
    if (uid == 0) return 1;
    

    if (node->uid == uid) {
        if ((node->mask & (access_mask << 6)) == (access_mask << 6)) {
            return 1;
        }
    }
    else if (node->gid == gid) {
        if ((node->mask & (access_mask << 3)) == (access_mask << 3)) {
            return 1;
        }
    }
    else {
        if ((node->mask & access_mask) == access_mask) {
            return 1;
        }
    }
    
    return 0;
}

fs_node_t* fs_create_symlink(char* name, fs_node_t* target) {
    if (!target) return NULL;
    
    fs_node_t* link = fs_create_node(name, FS_SYMLINK);
    if (!link) return NULL;
    
 
    link->symlink_target = target;
    
    link->readlink = fs_readlink;
    
    return link;
}

fs_node_t* fs_readlink(fs_node_t* link) {
    if (!link || link->type != FS_SYMLINK) return NULL;
    
    return link->symlink_target;
}

fs_node_t* fs_resolve_path(const char* path) {
    if (!path || !fs_root) return NULL;
    
    fs_node_t* current = fs_root;
    
    if (path[0] == '/') {
        path++; 
    }

    if (path[0] == '\0') {
        return fs_root; 
    }
    
    char component[128];
    int i = 0, j = 0;
    
    while (path[i] != '\0') {
        j = 0;
        while (path[i] != '/' && path[i] != '\0' && j < 127) {
            component[j++] = path[i++];
        }
        component[j] = '\0';
        
        if (j == 0) {
            if (path[i] == '/') i++;
            continue;
        }
        
        // .
        if (strcmp(component, ".") == 0) {
            if (path[i] == '/') i++;
            continue;
        }
        
        // ..
        if (strcmp(component, "..") == 0) {
            if (current->parent) {
                current = current->parent;
            }
            if (path[i] == '/') i++;
            continue;
        }
        

        fs_node_t* next = fs_finddir(current, component);
        if (!next) {
            return NULL; 
        }
        
 
        current = next;
        

        if (current->type == FS_SYMLINK) {
            current = fs_readlink(current);
            if (!current) return NULL;
        }
        
        if (path[i] == '/') i++;
    }
    
    return current;
}


fs_node_t* fs_get_parent_dir(const char* path) {
    if (!path || !fs_root) return NULL;
    

    const char* last_slash = strrchr(path, '/');
    if (!last_slash) {
        return fs_root; 
    }
    

    if (last_slash == path) {
        return fs_root;
    }
    

    char temp_path[256];
    int len = last_slash - path;
    if (len > 255) len = 255;
    

    int i;
    for (i = 0; i < len; i++) {
        temp_path[i] = path[i];
    }
    temp_path[i] = '\0';
    

    return fs_resolve_path(temp_path);
}


void fs_test(void) {
    terminal_writestring("Dosya sistemi testi yapiliyor...\n");
    

    fs_node_t* etc = fs_create_node("etc", FS_DIRECTORY);
    fs_node_t* home = fs_create_node("home", FS_DIRECTORY);
    fs_node_t* bin = fs_create_node("bin", FS_DIRECTORY);
    fs_node_t* usr = fs_create_node("usr", FS_DIRECTORY);
    

    fs_add_node(fs_root, etc);
    fs_add_node(fs_root, home);
    fs_add_node(fs_root, bin);
    fs_add_node(fs_root, usr);
    

    fs_node_t* passwd = fs_create_node("passwd", FS_FILE);
    fs_node_t* hosts = fs_create_node("hosts", FS_FILE);
    

    passwd->mask = FS_PERM_READ | (FS_PERM_READ << 3) | (FS_PERM_READ | FS_PERM_WRITE << 6);
    passwd->uid = 0;
    passwd->gid = 0;
    
    hosts->mask = FS_PERM_READ | (FS_PERM_READ << 3) | (FS_PERM_READ | FS_PERM_WRITE << 6);
    hosts->uid = 0;
    hosts->gid = 0;
    

    fs_add_node(etc, passwd);
    fs_add_node(etc, hosts);
    

    fs_node_t* user1 = fs_create_node("user1", FS_DIRECTORY);
    fs_add_node(home, user1);
    user1->uid = 1000;
    user1->gid = 1000;
    user1->mask = FS_PERM_READ | FS_PERM_WRITE | FS_PERM_EXEC | 
                 (FS_PERM_READ << 3) | 
                 (FS_PERM_READ | FS_PERM_WRITE | FS_PERM_EXEC << 6);
    

    fs_node_t* etc_link = fs_create_symlink("etc_link", etc);
    fs_add_node(user1, etc_link);
    
    terminal_writestring("Gelişmiş dosya sistemi yapısı oluşturuldu.\n");
    terminal_writestring("Dizinler: /, /etc, /home, /bin, /usr\n");
    terminal_writestring("Dosyalar: /etc/passwd, /etc/hosts\n");
    terminal_writestring("Sembolik bağlantılar: /home/user1/etc_link -> /etc\n");
    

    fs_node_t* test_node = fs_resolve_path("/home/user1/etc_link/passwd");
    if (test_node) {
        terminal_writestring("Sembolik bağlantı çözümlemesi başarılı: ");
        terminal_writestring(test_node->name);
        terminal_writestring(" dosyasına başarıyla erişildi.\n");
    }
    

    if (fs_check_permission(user1, FS_PERM_READ, 1000, 1000)) {
        terminal_writestring("user1 dizini okuma izni testi başarılı.\n");
    }
    
    terminal_writestring("Dosya sistemi testi tamamlandi.\n");
}
