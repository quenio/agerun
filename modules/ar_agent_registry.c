/**
 * @file ar_agent_registry.c
 * @brief Implementation of the agent registry module
 */

#include "ar_agent_registry.h"
#include "ar_data.h"
#include "ar_heap.h"
#include "ar_list.h"
#include "ar_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

/* Agent registry structure */
struct ar_agent_registry_s {
    ar_list_t *own_registered_ids;  // List of registered agent IDs
    ar_map_t *own_agent_map;        // Map of agent_id -> agent pointer
    int64_t next_agent_id;       // Next ID to allocate
};

/* Create a new agent registry */
ar_agent_registry_t* ar_agent_registry__create(void) {
    ar_agent_registry_t *own_registry = AR__HEAP__MALLOC(sizeof(ar_agent_registry_t), "agent registry");
    if (!own_registry) {
        return NULL;
    }
    
    own_registry->own_registered_ids = ar_list__create();
    if (!own_registry->own_registered_ids) {
        AR__HEAP__FREE(own_registry);
        return NULL;
    }
    
    own_registry->own_agent_map = ar_map__create();
    if (!own_registry->own_agent_map) {
        ar_list__destroy(own_registry->own_registered_ids);
        AR__HEAP__FREE(own_registry);
        return NULL;
    }
    
    own_registry->next_agent_id = 1;
    
    // Ownership transferred to caller
    return own_registry;
}

/* Destroy an agent registry */
void ar_agent_registry__destroy(ar_agent_registry_t *own_registry) {
    if (!own_registry) {
        return;
    }
    
    if (own_registry->own_registered_ids) {
        // Destroy all ID data in the list
        while (ar_list__count(own_registry->own_registered_ids) > 0) {
            ar_data_t *own_data = ar_list__remove_first(own_registry->own_registered_ids);
            if (own_data) {
                ar_data__destroy(own_data);
            }
        }
        ar_list__destroy(own_registry->own_registered_ids);
    }
    
    if (own_registry->own_agent_map) {
        ar_map__destroy(own_registry->own_agent_map);
    }
    
    AR__HEAP__FREE(own_registry);
}

/* Get the number of registered agents */
int ar_agent_registry__count(const ar_agent_registry_t *ref_registry) {
    if (!ref_registry || !ref_registry->own_registered_ids) {
        return 0;
    }
    return (int)ar_list__count(ref_registry->own_registered_ids);
}

/* Get the first registered agent ID */
int64_t ar_agent_registry__get_first(const ar_agent_registry_t *ref_registry) {
    if (!ref_registry || !ref_registry->own_registered_ids) {
        return 0;
    }
    
    if (ar_list__count(ref_registry->own_registered_ids) == 0) {
        return 0;
    }
    
    ar_data_t *ref_data = (ar_data_t*)ar_list__first(ref_registry->own_registered_ids);
    if (!ref_data) {
        return 0;
    }
    
    const char *id_str = ar_data__get_string(ref_data);
    if (!id_str) {
        return 0;
    }
    
    return strtoll(id_str, NULL, 10);
}

/* Get the next registered agent ID */
int64_t ar_agent_registry__get_next(const ar_agent_registry_t *ref_registry, int64_t current_id) {
    if (!ref_registry || !ref_registry->own_registered_ids) {
        return 0;
    }
    
    // Since we don't have indexed access to list, we need to iterate through all items
    void **items = ar_list__items(ref_registry->own_registered_ids);
    if (!items) {
        return 0;
    }
    
    size_t count = ar_list__count(ref_registry->own_registered_ids);
    bool found_current = false;
    int64_t next_id = 0;
    
    for (size_t i = 0; i < count; i++) {
        ar_data_t *ref_data = (ar_data_t*)items[i];
        if (!ref_data) {
            continue;
        }
        
        const char *id_str = ar_data__get_string(ref_data);
        if (!id_str) {
            continue;
        }
        
        int64_t id = strtoll(id_str, NULL, 10);
        if (found_current) {
            next_id = id;
            break;
        }
        if (id == current_id) {
            found_current = true;
        }
    }
    
    AR__HEAP__FREE(items);
    return next_id;
}

/* Clear all agents from the registry */
void ar_agent_registry__clear(ar_agent_registry_t *mut_registry) {
    if (!mut_registry || !mut_registry->own_registered_ids) {
        return;
    }
    
    // Destroy all ID data in the list
    while (ar_list__count(mut_registry->own_registered_ids) > 0) {
        ar_data_t *own_data = ar_list__remove_first(mut_registry->own_registered_ids);
        if (own_data) {
            ar_data__destroy(own_data);
        }
    }
    
    // Clear the agent map by destroying and recreating it
    if (mut_registry->own_agent_map) {
        ar_map__destroy(mut_registry->own_agent_map);
        mut_registry->own_agent_map = ar_map__create();
    }
    
    // Reset next agent ID
    mut_registry->next_agent_id = 1;
}

/* Get the next agent ID */
int64_t ar_agent_registry__get_next_id(const ar_agent_registry_t *ref_registry) {
    if (!ref_registry) {
        return 0;
    }
    return ref_registry->next_agent_id;
}

/* Set the next agent ID */
void ar_agent_registry__set_next_id(ar_agent_registry_t *mut_registry, int64_t id) {
    if (!mut_registry) {
        return;
    }
    mut_registry->next_agent_id = id;
}

/* Allocate a new agent ID */
int64_t ar_agent_registry__allocate_id(ar_agent_registry_t *mut_registry) {
    if (!mut_registry) {
        return 0;
    }
    return mut_registry->next_agent_id++;
}

/* Register an agent ID */
bool ar_agent_registry__register_id(ar_agent_registry_t *mut_registry, int64_t agent_id) {
    if (!mut_registry || !mut_registry->own_registered_ids) {
        return false;
    }
    
    // Check if already registered
    if (ar_agent_registry__is_registered(mut_registry, agent_id)) {
        return false;
    }
    
    // Create string representation of the ID
    char id_str[32];
    snprintf(id_str, sizeof(id_str), "%" PRId64, agent_id);
    
    // Create string data for the ID (ar_data__create_string copies the string)
    ar_data_t *own_id_data = ar_data__create_string(id_str);
    if (!own_id_data) {
        return false;
    }
    
    // Add to the list
    if (!ar_list__add_last(mut_registry->own_registered_ids, own_id_data)) {
        ar_data__destroy(own_id_data);
        return false;
    }
    
    return true;
}

/* Unregister an agent ID */
bool ar_agent_registry__unregister_id(ar_agent_registry_t *mut_registry, int64_t agent_id) {
    if (!mut_registry || !mut_registry->own_registered_ids) {
        return false;
    }
    
    // First, ensure the agent is untracked from the map
    // This must be done before we destroy the ID string
    ar_agent_registry__untrack_agent(mut_registry, agent_id);
    
    // Find and remove the ID from the list
    void **items = ar_list__items(mut_registry->own_registered_ids);
    if (!items) {
        return false;
    }
    
    size_t count = ar_list__count(mut_registry->own_registered_ids);
    ar_data_t *target = NULL;
    
    for (size_t i = 0; i < count; i++) {
        ar_data_t *ref_data = (ar_data_t*)items[i];
        if (!ref_data) {
            continue;
        }
        
        const char *id_str = ar_data__get_string(ref_data);
        if (!id_str) {
            continue;
        }
        
        int64_t id = strtoll(id_str, NULL, 10);
        if (id == agent_id) {
            target = ref_data;
            break;
        }
    }
    
    AR__HEAP__FREE(items);
    
    if (target) {
        ar_data_t *own_removed = (ar_data_t*)ar_list__remove(mut_registry->own_registered_ids, target);
        if (own_removed) {
            ar_data__destroy(own_removed);
        }
        return true;
    }
    
    return false;
}

/* Check if an agent ID is registered */
bool ar_agent_registry__is_registered(const ar_agent_registry_t *ref_registry, int64_t agent_id) {
    if (!ref_registry || !ref_registry->own_registered_ids) {
        return false;
    }
    
    void **items = ar_list__items(ref_registry->own_registered_ids);
    if (!items) {
        return false;
    }
    
    size_t count = ar_list__count(ref_registry->own_registered_ids);
    bool found = false;
    
    for (size_t i = 0; i < count; i++) {
        ar_data_t *ref_data = (ar_data_t*)items[i];
        if (!ref_data) {
            continue;
        }
        
        const char *id_str = ar_data__get_string(ref_data);
        if (!id_str) {
            continue;
        }
        
        int64_t id = strtoll(id_str, NULL, 10);
        if (id == agent_id) {
            found = true;
            break;
        }
    }
    
    AR__HEAP__FREE(items);
    return found;
}

/* Helper function to get string key for agent_id from the registered list */
static const char* _get_agent_key_from_list(const ar_agent_registry_t *ref_registry, int64_t agent_id) {
    if (!ref_registry || !ref_registry->own_registered_ids) {
        return NULL;
    }
    
    void **items = ar_list__items(ref_registry->own_registered_ids);
    if (!items) {
        return NULL;
    }
    
    size_t count = ar_list__count(ref_registry->own_registered_ids);
    const char *key = NULL;
    
    for (size_t i = 0; i < count; i++) {
        ar_data_t *ref_data = (ar_data_t*)items[i];
        if (!ref_data) {
            continue;
        }
        
        const char *id_str = ar_data__get_string(ref_data);
        if (!id_str) {
            continue;
        }
        
        int64_t id = strtoll(id_str, NULL, 10);
        if (id == agent_id) {
            key = id_str;
            break;
        }
    }
    
    AR__HEAP__FREE(items);
    return key;
}

/* Track an agent object in the registry */
bool ar_agent_registry__track_agent(ar_agent_registry_t *mut_registry, int64_t agent_id, void *mut_agent) {
    if (!mut_registry || !mut_registry->own_agent_map || !mut_agent) {
        return false;
    }
    
    // Get the string key from the registered list
    const char *ref_key = _get_agent_key_from_list(mut_registry, agent_id);
    if (!ref_key) {
        return false; // Agent ID must be registered first
    }
    
    // Store the agent pointer directly in the map
    // Note: We're storing the pointer value, not taking ownership
    return ar_map__set(mut_registry->own_agent_map, ref_key, mut_agent);
}

/* Untrack an agent object from the registry */
void* ar_agent_registry__untrack_agent(ar_agent_registry_t *mut_registry, int64_t agent_id) {
    if (!mut_registry || !mut_registry->own_agent_map) {
        return NULL;
    }
    
    // Get the string key from the registered list
    const char *ref_key = _get_agent_key_from_list(mut_registry, agent_id);
    if (!ref_key) {
        return NULL;
    }
    
    // Get the agent pointer before removing
    void *agent = ar_map__get(mut_registry->own_agent_map, ref_key);
    if (agent) {
        // Remove by setting to NULL
        ar_map__set(mut_registry->own_agent_map, ref_key, NULL);
    }
    
    return agent;
}

/* Find a tracked agent by ID */
void* ar_agent_registry__find_agent(const ar_agent_registry_t *ref_registry, int64_t agent_id) {
    if (!ref_registry || !ref_registry->own_agent_map) {
        return NULL;
    }
    
    // Get the string key from the registered list
    const char *ref_key = _get_agent_key_from_list(ref_registry, agent_id);
    if (!ref_key) {
        return NULL;
    }
    
    return ar_map__get(ref_registry->own_agent_map, ref_key);
}
