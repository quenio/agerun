/**
 * @file ar_agent_store.c
 * @brief Implementation of the agent store module
 */

#include "ar_agent_store.h"
#include "ar_agent.h"
#include "ar_agent_registry.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_data.h"
#include "ar_list.h"
#include "ar_io.h"
#include "ar_heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <unistd.h>

/* Maximum reasonable limits */
#define MAX_STORE_AGENTS 10000
#define MAX_LINE_LENGTH 256
#define MAX_MEMORY_ITEMS 1000

/* Agent store structure */
struct ar_agent_store_s {
    ar_agent_registry_t *ref_registry;      /* Borrowed reference to registry */
    ar_methodology_t *ref_methodology;      /* Borrowed reference to methodology */
    const char *filename;                   /* Store filename */
};

/* Context structure for save operations */
typedef struct {
    const char *filename;
} store_save_context_t;

/* Helper function to get list of all active agent IDs */
static ar_list_t* _get_active_agent_list(ar_agent_registry_t *ref_registry) {
    ar_list_t *own_list = ar_list__create();
    if (!own_list) {
        return NULL;
    }
    
    int64_t agent_id = ar_agent_registry__get_first(ref_registry);
    while (agent_id != 0) {
        ar_data_t *own_id_data = ar_data__create_integer((int)agent_id);
        if (own_id_data) {
            ar_list__add_last(own_list, own_id_data);
        }
        agent_id = ar_agent_registry__get_next(ref_registry, agent_id);
    }
    
    return own_list;
}

/* Helper function to get agent method info as strings */
static bool _get_agent_method_info(ar_agent_registry_t *ref_registry, int64_t agent_id, 
                                  const char **out_method_name, const char **out_version) {
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(ref_registry, agent_id);
    if (!ref_agent) {
        return false;
    }
    
    const ar_method_t *ref_method = ar_agent__get_method(ref_agent);
    if (!ref_method) {
        return false;
    }
    
    *out_method_name = ar_method__get_name(ref_method);
    *out_version = ar_method__get_version(ref_method);
    return true;
}

/* Helper function to get agent memory */
static ar_data_t* _get_agent_memory(ar_agent_registry_t *ref_registry, int64_t agent_id) {
    ar_agent_t *ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(ref_registry, agent_id);
    if (!ref_agent) {
        return NULL;
    }
    
    return ar_agent__get_mutable_memory(ref_agent);
}

/* Helper function to create backup file */
static bool _create_backup(const char *filename) {
    if (!filename) {
        return false;
    }
    
    /* Check if original file exists using access() */
    if (access(filename, F_OK) != 0) {
        return true; /* No backup needed - file doesn't exist */
    }
    
    /* Use ar_io backup functionality */
    ar_file_result_t result = ar_io__create_backup(filename);
    return (result == AR_FILE_RESULT__SUCCESS);
}

/* Helper to save agent memory data to file */
static bool _save_agent_memory(FILE *file, ar_data_t *ref_memory) {
    if (!ref_memory || ar_data__get_type(ref_memory) != AR_DATA_TYPE__MAP) {
        fprintf(file, "memory_items=0\n");
        return true; /* No memory or not a map - nothing to save */
    }
    
    ar_data_t *own_keys = ar_data__get_map_keys(ref_memory);
    if (!own_keys) {
        fprintf(file, "memory_items=0\n");
        return true; /* No keys - nothing to save */
    }
    
    size_t key_count = ar_data__list_count(own_keys);
    if (key_count > MAX_MEMORY_ITEMS) {
        ar_data__destroy(own_keys);
        return false;
    }
    
    fprintf(file, "memory_items=%zu\n", key_count);
    
    /* Save each memory item using simple approach - skip iteration for now */
    /* TODO: Implement proper memory saving once key iteration is working */
    (void)key_count; /* Suppress unused variable warning */
    
    ar_data__destroy(own_keys);
    return true;
}

/* Create a new agent store instance */
ar_agent_store_t* ar_agent_store__create(ar_agent_registry_t *ref_registry, ar_methodology_t *ref_methodology) {
    if (!ref_registry || !ref_methodology) {
        return NULL;
    }
    
    ar_agent_store_t *own_store = AR__HEAP__MALLOC(sizeof(ar_agent_store_t), "agent store");
    if (!own_store) {
        return NULL;
    }
    
    own_store->ref_registry = ref_registry;      /* Store borrowed reference */
    own_store->ref_methodology = ref_methodology; /* Store borrowed reference */
    own_store->filename = AGENT_STORE_FILE_NAME;
    
    return own_store;
}

/* Destroy an agent store instance */
void ar_agent_store__destroy(ar_agent_store_t *own_store) {
    if (!own_store) {
        return;
    }
    
    AR__HEAP__FREE(own_store);
}

/* Save all agents to persistent storage */
bool ar_agent_store__save(ar_agent_store_t *ref_store) {
    if (!ref_store || !ref_store->ref_registry) {
        return false;
    }
    
    /* Get list of active agents */
    ar_list_t *own_agent_list = _get_active_agent_list(ref_store->ref_registry);
    if (!own_agent_list) {
        return false;
    }
    
    size_t agent_count_size = ar_list__count(own_agent_list);
    if (agent_count_size > MAX_STORE_AGENTS) {
        ar_list__destroy(own_agent_list);
        return false;
    }
    int agent_count = (int)agent_count_size;
    
    /* Create backup */
    _create_backup(ref_store->filename);
    
    /* Open file for writing */
    FILE *file = NULL;
    ar_file_result_t open_result = ar_io__open_file(ref_store->filename, "w", &file);
    if (open_result != AR_FILE_RESULT__SUCCESS || !file) {
        ar_list__destroy(own_agent_list);
        return false;
    }
    
    /* Write header */
    fprintf(file, "# AgeRun Agent Store\n");
    fprintf(file, "version=1\n");
    fprintf(file, "agent_count=%d\n", agent_count);
    
    /* Get array of items for iteration */
    void **items = ar_list__items(own_agent_list);
    if (!items && agent_count > 0) {
        ar_io__close_file(file, ref_store->filename);
        ar_list__destroy(own_agent_list);
        return false;
    }
    
    /* Write each agent */
    for (int i = 0; i < agent_count; i++) {
        ar_data_t *ref_id_data = (ar_data_t*)items[i];
        if (!ref_id_data) continue;
        
        int64_t agent_id = ar_data__get_integer(ref_id_data);
        
        /* Get method info */
        const char *method_name = NULL;
        const char *version = NULL;
        if (!_get_agent_method_info(ref_store->ref_registry, agent_id, &method_name, &version)) {
            continue;
        }
        
        fprintf(file, "\n# Agent %" PRId64 "\n", agent_id);
        fprintf(file, "agent_id=%" PRId64 "\n", agent_id);
        fprintf(file, "method_name=%s\n", method_name ? method_name : "");
        fprintf(file, "method_version=%s\n", version ? version : "");
        
        /* Save agent memory */
        ar_data_t *ref_memory = _get_agent_memory(ref_store->ref_registry, agent_id);
        _save_agent_memory(file, ref_memory);
    }
    
    /* Free the items array and the data objects it contains */
    if (items) {
        for (int i = 0; i < agent_count; i++) {
            ar_data_t *own_id_data = (ar_data_t*)items[i];
            if (own_id_data) {
                ar_data__destroy(own_id_data);
            }
        }
        AR__HEAP__FREE(items);
    }
    
    ar_io__close_file(file, ref_store->filename);
    ar_list__destroy(own_agent_list);
    
    int count = ar_agent_registry__count(ref_store->ref_registry);
    ar_io__info("Successfully saved %d agents to file", count);
    return true;
}

/* Load all agents from persistent storage */
bool ar_agent_store__load(ar_agent_store_t *mut_store) {
    if (!mut_store || !mut_store->ref_registry) {
        return false;
    }
    
    /* Check if file exists using access() */
    if (access(mut_store->filename, F_OK) != 0) {
        ar_io__info("Warning: Agent store file does not exist");
        return true; /* Not an error if file doesn't exist */
    }
    
    /* Open file for reading */
    FILE *file = NULL;
    ar_file_result_t open_result = ar_io__open_file(mut_store->filename, "r", &file);
    if (open_result != AR_FILE_RESULT__SUCCESS || !file) {
        return false;
    }
    
    char line[MAX_LINE_LENGTH];
    int agent_count = 0;
    
    /* Parse file and load agents */
    while (ar_io__read_line(file, line, sizeof(line), mut_store->filename)) {
        if (strncmp(line, "agent_count=", 12) == 0) {
            agent_count = (int)strtol(line + 12, NULL, 10);
            if (agent_count > MAX_STORE_AGENTS) {
                ar_io__close_file(file, mut_store->filename);
                return false;
            }
        }
        /* Additional parsing logic would go here */
    }
    
    ar_io__close_file(file, mut_store->filename);
    return true;
}

/* Check if agent store file exists */
bool ar_agent_store__exists(ar_agent_store_t *ref_store) {
    if (!ref_store) {
        return false;
    }
    
    return (access(ref_store->filename, F_OK) == 0);
}

/* Delete the agent store file */
bool ar_agent_store__delete(ar_agent_store_t *ref_store) {
    if (!ref_store) {
        return false;
    }
    
    if (access(ref_store->filename, F_OK) != 0) {
        return true; /* Nothing to delete - file doesn't exist */
    }
    
    _create_backup(ref_store->filename);
    /* Use standard C remove() function */
    return (remove(ref_store->filename) == 0);
}

/* Get the path to the agent store file */
const char* ar_agent_store__get_path(ar_agent_store_t *ref_store) {
    if (!ref_store) {
        return NULL;
    }
    
    return ref_store->filename;
}

/* Get the methodology reference from agent store */
ar_methodology_t* ar_agent_store__get_methodology(ar_agent_store_t *ref_store) {
    if (!ref_store) {
        return NULL;
    }
    
    return ref_store->ref_methodology;
}