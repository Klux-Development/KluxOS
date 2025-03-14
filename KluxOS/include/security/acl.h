#ifndef ACL_H
#define ACL_H

#include <kernel/types.h>
#include <kernel/fs.h>

// Erişim kontrolü için kullanıcı tipleri
typedef enum {
    ACL_USER_ROOT = 0,
    ACL_USER_SYSTEM,
    ACL_USER_NORMAL,
    ACL_USER_GUEST
} acl_user_type_t;

// Erişim izni tipleri
typedef enum {
    ACL_PERM_NONE = 0,
    ACL_PERM_READ = 1,
    ACL_PERM_WRITE = 2,
    ACL_PERM_EXEC = 4,
    ACL_PERM_ALL = 7
} acl_permission_t;

// ACL girişi
typedef struct {
    uint32_t user_id;
    uint32_t group_id;
    acl_permission_t permissions;
} acl_entry_t;

// Kaynak tipleri
typedef enum {
    ACL_RES_FILE,
    ACL_RES_PROCESS,
    ACL_RES_MEMORY,
    ACL_RES_NETWORK
} acl_resource_type_t;

// Kaynak yapısı
typedef struct {
    acl_resource_type_t type;
    void* resource;           // Kaynak işaretçisi (fs_node_t*, process_t*, vb.)
    acl_entry_t* acl;         // ACL girişi
    uint32_t acl_count;       // ACL giriş sayısı
} acl_resource_t;

// Güvenlik modülünü başlat
void security_init(void);

// ACL işlemleri
int acl_add_entry(acl_resource_t* resource, uint32_t user_id, uint32_t group_id, acl_permission_t perms);
int acl_remove_entry(acl_resource_t* resource, uint32_t user_id, uint32_t group_id);
int acl_check_permission(acl_resource_t* resource, uint32_t user_id, uint32_t group_id, acl_permission_t perm);

// Kaynak koruma
int acl_protect_file(fs_node_t* file, uint32_t user_id, uint32_t group_id, acl_permission_t perms);
int acl_protect_process(void* process, uint32_t user_id, uint32_t group_id, acl_permission_t perms);
int acl_protect_memory(void* memory, size_t size, uint32_t user_id, uint32_t group_id, acl_permission_t perms);

// Güvenlik denetimi
void security_audit_log(const char* message, uint32_t user_id, uint32_t resource_id, acl_permission_t perm);

#endif
