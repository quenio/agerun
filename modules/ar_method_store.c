/**
 * @file ar_method_store.c
 * @brief Implementation of the method store module
 */

#include "ar_method_store.h"
#include "ar_method_registry.h"
#include "ar_method.h"
#include "ar_list.h"
#include "ar_heap.h"
#include "ar_io.h"
#include "ar_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Method store structure */
struct ar_method_store_s {
    ar_method_registry_t *ref_registry;  /* Borrowed reference to method registry */
    char *own_file_path;                 /* Owned file path for persistence */
    ar_log_t *ref_log;                   /* Borrowed reference to log for error reporting */
};

/* Internal structure for grouping methods by name */
typedef struct {
    const char *name;
    int count;
    ar_method_t **methods;
} method_group_t;

ar_method_store_t* ar_method_store__create(ar_log_t *ref_log, ar_method_registry_t *ref_registry, const char *ref_file_path) {
    if (!ref_registry || !ref_file_path) {
        return NULL;
    }
    
    ar_method_store_t *own_store = AR__HEAP__MALLOC(sizeof(ar_method_store_t), "method store");
    if (!own_store) {
        return NULL;
    }
    
    own_store->ref_log = ref_log;
    own_store->ref_registry = ref_registry;
    
    /* Copy the file path */
    own_store->own_file_path = AR__HEAP__STRDUP(ref_file_path, "method store file path");
    if (!own_store->own_file_path) {
        AR__HEAP__FREE(own_store);
        return NULL;
    }
    
    return own_store;
}

void ar_method_store__destroy(ar_method_store_t *own_store) {
    if (!own_store) {
        return;
    }
    
    if (own_store->own_file_path) {
        AR__HEAP__FREE(own_store->own_file_path);
    }
    AR__HEAP__FREE(own_store);
}

const char* ar_method_store__get_path(ar_method_store_t *ref_store) {
    if (!ref_store) {
        return NULL;
    }
    return ref_store->own_file_path;
}

bool ar_method_store__exists(ar_method_store_t *ref_store) {
    if (!ref_store || !ref_store->own_file_path) {
        return false;
    }
    
    /* Use fopen directly for consistency */
    FILE *fp = fopen(ref_store->own_file_path, "r");
    if (fp != NULL) {
        fclose(fp);
        return true;
    }
    return false;
}

/* Helper to clean up method groups */
static void _cleanup_method_groups(method_group_t *groups, int group_count) {
    if (groups) {
        for (int i = 0; i < group_count; i++) {
            if (groups[i].methods) {
                AR__HEAP__FREE(groups[i].methods);
            }
        }
        AR__HEAP__FREE(groups);
    }
}

bool ar_method_store__save(ar_method_store_t *ref_store) {
    if (!ref_store || !ref_store->own_file_path) {
        return false;
    }
    
    /* Use fopen directly to avoid ar_io issues with file creation */
    FILE *fp = fopen(ref_store->own_file_path, "w");
    if (fp != NULL) {
        /* Get all methods from the registry */
        ar_list_t *own_methods = ar_method_registry__get_all_methods(ref_store->ref_registry);
        if (!own_methods) {
            fclose(fp);
            return false;
        }
        
        size_t total_count = ar_list__count(own_methods);
        void **own_items = ar_list__items(own_methods);
        
        /* If no methods, just write the count (0) and return */
        if (total_count == 0 || !own_items) {
            fprintf(fp, "0\n");
            ar_list__destroy(own_methods);
            fclose(fp);
            return true;
        }
        
        /* Build groups of methods by name - matching ar_methodology exactly */
        int group_capacity = 16;
        method_group_t *groups = AR__HEAP__MALLOC(sizeof(method_group_t) * (size_t)group_capacity, "method groups");
        if (!groups) {
            AR__HEAP__FREE(own_items);
            ar_list__destroy(own_methods);
            fclose(fp);
            return false;
        }
        int group_count = 0;
        
        /* Group methods by name */
        for (size_t i = 0; i < total_count; i++) {
            ar_method_t *method = (ar_method_t *)own_items[i];
            if (!method) continue;
            
            const char *name = ar_method__get_name(method);
            
            /* Find or create group */
            int group_idx = -1;
            for (int j = 0; j < group_count; j++) {
                if (strcmp(groups[j].name, name) == 0) {
                    group_idx = j;
                    break;
                }
            }
            
            if (group_idx < 0) {
                /* New group */
                if (group_count >= group_capacity) {
                    /* Grow groups array */
                    group_capacity *= 2;
                    method_group_t *new_groups = AR__HEAP__REALLOC(groups, 
                        sizeof(method_group_t) * (size_t)group_capacity, "method groups");
                    if (!new_groups) {
                        _cleanup_method_groups(groups, group_count);
                        AR__HEAP__FREE(own_items);
                        ar_list__destroy(own_methods);
                        fclose(fp);
                        return false;
                    }
                    groups = new_groups;
                }
                
                group_idx = group_count++;
                groups[group_idx].name = name;
                groups[group_idx].count = 0;
                groups[group_idx].methods = AR__HEAP__MALLOC(
                    sizeof(ar_method_t*) * 32, "method versions");
                if (!groups[group_idx].methods) {
                    _cleanup_method_groups(groups, group_count - 1);
                    AR__HEAP__FREE(own_items);
                    ar_list__destroy(own_methods);
                    fclose(fp);
                    return false;
                }
            }
            
            /* Add method to group */
            /* TODO: In future, add dynamic growing for methods array if count exceeds 32 */
            groups[group_idx].methods[groups[group_idx].count++] = method;
        }
        
        /* Write method count (unique names) */
        fprintf(fp, "%d\n", group_count);
        
        /* Write each method group */
        for (int i = 0; i < group_count; i++) {
            /* Write method name and version count */
            fprintf(fp, "%s %d\n", groups[i].name, groups[i].count);
            
            /* Write each version */
            for (int j = 0; j < groups[i].count; j++) {
                ar_method_t *method = groups[i].methods[j];
                const char *version = ar_method__get_version(method);
                const char *instructions = ar_method__get_instructions(method);
                
                /* Write version */
                fprintf(fp, "%s\n", version);
                
                /* Write instructions */
                fprintf(fp, "%s\n", instructions ? instructions : "");
            }
        }
        
        /* Clean up */
        _cleanup_method_groups(groups, group_count);
        AR__HEAP__FREE(own_items);
        ar_list__destroy(own_methods);
        
        fclose(fp);
        return true;
    }
    return false;
}


bool ar_method_store__load(ar_method_store_t *mut_store) {
    if (!mut_store || !mut_store->own_file_path) {
        return false;
    }
    
    /* Check if file exists */
    if (!ar_method_store__exists(mut_store)) {
        return false;
    }
    
    /* Open file for reading */
    FILE *fp = fopen(mut_store->own_file_path, "r");
    if (!fp) {
        return false;
    }
    
    /* Read method count */
    int method_count = 0;
    if (fscanf(fp, "%d\n", &method_count) != 1) {
        fclose(fp);
        return false;
    }
    
    /* For empty file (count = 0), just close and return success */
    if (method_count == 0) {
        fclose(fp);
        return true;
    }
    
    /* Load methods - matching ar_methodology format */
    for (int i = 0; i < method_count; i++) {
        char method_name[256];
        int version_count = 0;
        
        /* Read method name and version count */
        if (fscanf(fp, "%255s %d\n", method_name, &version_count) != 2) {
            fclose(fp);
            return false;
        }
        
        /* Load each version */
        for (int j = 0; j < version_count; j++) {
            char version[256];
            char instructions[4096];
            
            /* Read version */
            if (fgets(version, sizeof(version), fp) == NULL) {
                fclose(fp);
                return false;
            }
            /* Remove newline */
            size_t len = strlen(version);
            if (len > 0 && version[len-1] == '\n') {
                version[len-1] = '\0';
            }
            
            /* Read instructions */
            if (fgets(instructions, sizeof(instructions), fp) == NULL) {
                fclose(fp);
                return false;
            }
            /* Remove newline */
            len = strlen(instructions);
            if (len > 0 && instructions[len-1] == '\n') {
                instructions[len-1] = '\0';
            }
            
            /* Create and register method */
            ar_method_t *own_method;
            if (mut_store->ref_log) {
                /* Use the log-aware create function if we have a log */
                own_method = ar_method__create_with_log(method_name, instructions, version, mut_store->ref_log);
            } else {
                /* Use the standard create function if no log */
                own_method = ar_method__create(method_name, instructions, version);
            }
            if (!own_method) {
                fclose(fp);
                return false;
            }
            
            /* Register with the registry */
            ar_method_registry__register_method(mut_store->ref_registry, own_method);
        }
    }
    
    fclose(fp);
    return true;
}

bool ar_method_store__delete(ar_method_store_t *ref_store) {
    if (!ref_store || !ref_store->own_file_path) {
        return false;
    }
    
    /* For now, just try to remove the file */
    remove(ref_store->own_file_path);
    return true;
}