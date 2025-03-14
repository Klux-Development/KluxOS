#include <security/acl.h>
#include <kernel/types.h>
#include <drivers/terminal.h>
#include "../kernel/mm/memory.h"

void security_init(void) {
    terminal_writestring("security init started\n");

    terminal_writestring("access control list (ACL) init started\n");
    
    terminal_writestring("security audit log init started\n");
    
    terminal_writestring("security init completed\n");
}


int acl_add_entry(acl_resource_t* resource, uint32_t user_id, uint32_t group_id, acl_permission_t perms) {
    if (!resource) return -1;
    
    acl_entry_t* new_acl = (acl_entry_t*)kmalloc((resource->acl_count + 1) * sizeof(acl_entry_t));
    if (!new_acl) return -1;
    
    for (uint32_t i = 0; i < resource->acl_count; i++) {
        new_acl[i] = resource->acl[i];
    }
    
    new_acl[resource->acl_count].user_id = user_id;
    new_acl[resource->acl_count].group_id = group_id;
    new_acl[resource->acl_count].permissions = perms;
    
    if (resource->acl) {
        kfree(resource->acl);
    }
    
    resource->acl = new_acl;
    resource->acl_count++;
    
    return 0;
}


int acl_remove_entry(acl_resource_t* resource, uint32_t user_id, uint32_t group_id) {
    if (!resource || !resource->acl || resource->acl_count == 0) return -1;
    
    int found_idx = -1;
    for (uint32_t i = 0; i < resource->acl_count; i++) {
        if (resource->acl[i].user_id == user_id && resource->acl[i].group_id == group_id) {
            found_idx = i;
            break;
        }
    }
    
    if (found_idx < 0) return -1;
    
    if (resource->acl_count == 1) {
        kfree(resource->acl);
        resource->acl = NULL;
        resource->acl_count = 0;
        return 0;
    }
    
    acl_entry_t* new_acl = (acl_entry_t*)kmalloc((resource->acl_count - 1) * sizeof(acl_entry_t));
    if (!new_acl) return -1;
    
    uint32_t new_idx = 0;
    for (uint32_t i = 0; i < resource->acl_count; i++) {
        if (i != found_idx) {
            new_acl[new_idx++] = resource->acl[i];
        }
    }
    
    kfree(resource->acl);
    
    resource->acl = new_acl;
    resource->acl_count--;
    
    return 0;
}


int acl_check_permission(acl_resource_t* resource, uint32_t user_id, uint32_t group_id, acl_permission_t perm) {
    if (!resource || !resource->acl) return 0;
    
    if (user_id == ACL_USER_ROOT) return 1;
    
    for (uint32_t i = 0; i < resource->acl_count; i++) {
        if (resource->acl[i].user_id == user_id || resource->acl[i].group_id == group_id) {
            if ((resource->acl[i].permissions & perm) == perm) {
                return 1;
            }
        }
    }
    
    return 0;
}


int acl_protect_file(fs_node_t* file, uint32_t user_id, uint32_t group_id, acl_permission_t perms) {
    if (!file) return -1;
    
    acl_resource_t* resource = (acl_resource_t*)kmalloc(sizeof(acl_resource_t));
    if (!resource) return -1;
    
    resource->type = ACL_RES_FILE;
    resource->resource = file;
    resource->acl = NULL;
    resource->acl_count = 0;
    
    if (acl_add_entry(resource, user_id, group_id, perms) < 0) {
        kfree(resource);
        return -1;
    }
    
    file->mask = 0;
    if (perms & ACL_PERM_READ) file->mask |= FS_PERM_READ;
    if (perms & ACL_PERM_WRITE) file->mask |= FS_PERM_WRITE;
    if (perms & ACL_PERM_EXEC) file->mask |= FS_PERM_EXEC;
    
    file->uid = user_id;
    file->gid = group_id;
    
    return 0;
}


int acl_protect_process(void* process, uint32_t user_id, uint32_t group_id, acl_permission_t perms) {
    if (!process) return -1;

    acl_resource_t* resource = (acl_resource_t*)kmalloc(sizeof(acl_resource_t));
    if (!resource) return -1;
    
    resource->type = ACL_RES_PROCESS;
    resource->resource = process;
    resource->acl = NULL;
    resource->acl_count = 0;
    
    if (acl_add_entry(resource, user_id, group_id, perms) < 0) {
        kfree(resource);
        return -1;
    }
    
    return 0;
}


int acl_protect_memory(void* memory, size_t size, uint32_t user_id, uint32_t group_id, acl_permission_t perms) {
    if (!memory) return -1;
    
    acl_resource_t* resource = (acl_resource_t*)kmalloc(sizeof(acl_resource_t));
    if (!resource) return -1;
    
    resource->type = ACL_RES_MEMORY;
    resource->resource = memory;
    resource->acl = NULL;
    resource->acl_count = 0;
    
    if (acl_add_entry(resource, user_id, group_id, perms) < 0) {
        kfree(resource);
        return -1;
    }
    
    return 0;
}


void security_audit_log(const char* message, uint32_t user_id, uint32_t resource_id, acl_permission_t perm) {
    terminal_writestring("[GUVENLIK DENETIMI] ");
    terminal_writestring(message);
    terminal_writestring(" (Kullanici: ");
    
    char user_id_str[10];
    int i = 0;
    uint32_t temp = user_id;
    do {
        user_id_str[i++] = (temp % 10) + '0';
        temp /= 10;
    } while (temp > 0);
    user_id_str[i] = '\0';
    
    for (int j = 0; j < i / 2; j++) {
        char tmp = user_id_str[j];
        user_id_str[j] = user_id_str[i - j - 1];
        user_id_str[i - j - 1] = tmp;
    }
    
    terminal_writestring(user_id_str);
    terminal_writestring(")\n");
}
