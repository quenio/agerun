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
#include "ar_yaml_writer.h"
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

/* Helper to create backup file */
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

/* Helper function to build YAML root structure */
static ar_data_t* _build_yaml_root_structure(void) {
    /* Create root map */
    ar_data_t *own_root = ar_data__create_map();
    if (!own_root) {
        return NULL;
    }
    
    /* Create agents list */
    ar_data_t *own_agents_list = ar_data__create_list();
    if (!own_agents_list) {
        ar_data__destroy(own_root);
        return NULL;
    }
    
    /* Add agents list to root */
    ar_data__set_map_data(own_root, "agents", own_agents_list);
    return own_root;
}

/* Helper function to copy agent memory to YAML format */
static ar_data_t* _copy_agent_memory_to_yaml(ar_data_t *ref_memory) {
    if (!ref_memory || ar_data__get_type(ref_memory) != AR_DATA_TYPE__MAP) {
        /* Empty memory map */
        return ar_data__create_map();
    }
    
    /* Create a fresh memory map and copy key-value pairs */
    ar_data_t *own_memory_map = ar_data__create_map();
    if (!own_memory_map) {
        return NULL;
    }
    
    ar_data_t *own_keys = ar_data__get_map_keys(ref_memory);
    if (own_keys) {
        size_t key_count = ar_data__list_count(own_keys);
        if (key_count <= MAX_MEMORY_ITEMS) {
            ar_data_t **key_items = ar_data__list_items(own_keys);
            for (size_t k = 0; k < key_count; k++) {
                const char *key = ar_data__get_string(key_items[k]);
                if (key) {
                    ar_data_t *ref_value = ar_data__get_map_data(ref_memory, key);
                    if (ref_value) {
                        // Copy based on type
                        if (ar_data__get_type(ref_value) == AR_DATA_TYPE__STRING) {
                            ar_data__set_map_string(own_memory_map, key, ar_data__get_string(ref_value));
                        } else if (ar_data__get_type(ref_value) == AR_DATA_TYPE__INTEGER) {
                            ar_data__set_map_integer(own_memory_map, key, ar_data__get_integer(ref_value));
                        } else if (ar_data__get_type(ref_value) == AR_DATA_TYPE__DOUBLE) {
                            ar_data__set_map_double(own_memory_map, key, ar_data__get_double(ref_value));
                        }
                        // Skip complex types for now (maps, lists)
                    }
                }
            }
            if (key_items) AR__HEAP__FREE(key_items);
        }
        ar_data__destroy(own_keys);
    }
    
    return own_memory_map;
}

/* Helper function to build YAML data for a single agent */
static ar_data_t* _build_agent_yaml_data(ar_agent_registry_t *ref_registry, int64_t agent_id) {
    /* Get method info */
    const char *method_name = NULL;
    const char *version = NULL;
    if (!_get_agent_method_info(ref_registry, agent_id, &method_name, &version)) {
        return NULL;
    }
    
    /* Create agent map */
    ar_data_t *own_agent_map = ar_data__create_map();
    if (!own_agent_map) {
        return NULL;
    }
    
    /* Add agent properties */
    ar_data__set_map_integer(own_agent_map, "id", (int)agent_id);
    if (method_name) {
        ar_data__set_map_string(own_agent_map, "method_name", method_name);
    }
    if (version) {
        ar_data__set_map_string(own_agent_map, "method_version", version);
    }
    
    /* Add agent memory */
    ar_data_t *ref_memory = _get_agent_memory(ref_registry, agent_id);
    ar_data_t *own_memory_map = _copy_agent_memory_to_yaml(ref_memory);
    if (own_memory_map) {
        ar_data__set_map_data(own_agent_map, "memory", own_memory_map);
    }
    
    return own_agent_map;
}

/* Helper function to write YAML data to file */
static bool _write_yaml_to_file(ar_data_t *ref_root, const char *filename) {
    ar_yaml_writer_t *own_writer = ar_yaml_writer__create(NULL);
    if (!own_writer) {
        return false;
    }
    
    bool success = ar_yaml_writer__write_to_file(own_writer, ref_root, filename);
    ar_yaml_writer__destroy(own_writer);
    
    return success;
}

/* Helper function to clean up agent list resources */
static void _cleanup_agent_list_resources(ar_list_t *own_agent_list, void **items, int agent_count) {
    if (items) {
        for (int i = 0; i < agent_count; i++) {
            ar_data_t *own_id_data = (ar_data_t*)items[i];
            if (own_id_data) {
                ar_data__destroy(own_id_data);
            }
        }
        AR__HEAP__FREE(items);
    }
    ar_list__destroy(own_agent_list);
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
    
    /* Get and validate agent list */
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
    if (!_create_backup(ref_store->filename)) {
        ar_list__destroy(own_agent_list);
        return false;
    }
    
    /* Build YAML root structure */
    ar_data_t *own_root = _build_yaml_root_structure();
    if (!own_root) {
        ar_list__destroy(own_agent_list);
        return false;
    }
    
    /* Get agents list and prepare for iteration */
    ar_data_t *ref_agents_list = ar_data__get_map_data(own_root, "agents");
    void **items = ar_list__items(own_agent_list);
    if (!items && agent_count > 0) {
        ar_data__destroy(own_root);
        ar_list__destroy(own_agent_list);
        return false;
    }
    
    /* Build agent data structures */
    for (int i = 0; i < agent_count; i++) {
        int64_t agent_id = ar_data__get_integer((ar_data_t*)items[i]);
        ar_data_t *own_agent_map = _build_agent_yaml_data(ref_store->ref_registry, agent_id);
        if (own_agent_map) {
            ar_data__list_add_last_data(ref_agents_list, own_agent_map);
        }
    }
    
    /* Write to file and cleanup */
    bool success = _write_yaml_to_file(own_root, ref_store->filename);
    ar_data__destroy(own_root);
    _cleanup_agent_list_resources(own_agent_list, items, agent_count);
    
    if (success) {
        ar_io__info("Successfully saved %d agents to YAML file", agent_count);
    }
    
    return success;
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
