/* Agerun Agent Implementation */
#include "agerun_agent.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
#include "agerun_agent_registry.h"
#include "agerun_list.h"
#include "agerun_heap.h"
#include "agerun_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Agent Definition (moved from header for opaque type) */
struct agent_s {
    int64_t id;
    const method_t *ref_method; // Borrowed reference to method
    bool is_active;
    list_t *own_message_queue;  // Using list as a message queue, owned by agent
    data_t *own_memory;        // Memory owned by agent
    const data_t *ref_context;  // Context is read-only reference, not owned
};

/* Constants */

/* Global State - moved from agency module */
static agent_registry_t *g_own_registry = NULL; // Owned by the agent module
static bool g_is_initialized = false;

/* Static variables for commonly used messages */
static const char g_sleep_message[] = "__sleep__";

/* Static initialization */
static void ar_agent_init(void) {
    if (!g_is_initialized) {
        g_own_registry = ar_agent_registry_create();
        if (!g_own_registry) {
            printf("Error: Failed to create agent registry\n");
            return;
        }
        g_is_initialized = true;
    }
}

/* Registry accessor */
agent_registry_t* ar_agent_get_registry(void) {
    if (!g_is_initialized) {
        ar_agent_init();
    }
    return g_own_registry;
}

/* Implementation */
int64_t ar_agent_create(const char *ref_method_name, const char *ref_version, const data_t *ref_context) {
    if (!g_is_initialized) {
        ar_agent_init();
    }
    
    if (!ref_method_name || !g_own_registry) {
        // No need to free ref_context as we don't own it
        return 0;
    }
    
    // Lookup method using methodology
    const method_t *ref_method = ar_methodology_get_method(ref_method_name, ref_version);
    if (!ref_method) {
        // No need to free ref_context as we don't own it
        return 0;  // Method not found
    }
    
    // Allocate a new agent ID
    int64_t new_id = ar_agent_registry_allocate_id(g_own_registry);
    if (new_id == 0) {
        return 0;  // Failed to allocate ID
    }
    
    // Create agent structure
    agent_t *own_agent = AR_HEAP_MALLOC(sizeof(agent_t), "agent");
    if (!own_agent) {
        return 0;
    }
    
    own_agent->id = new_id;
    own_agent->is_active = true;
    own_agent->ref_method = ref_method;  // Just store reference
    own_agent->ref_context = ref_context;  // Store reference, we don't own it
    
    own_agent->own_message_queue = ar_list_create();
    if (!own_agent->own_message_queue) {
        AR_HEAP_FREE(own_agent);
        return 0;
    }
    
    own_agent->own_memory = ar_data_create_map();
    if (!own_agent->own_memory) {
        ar_list_destroy(own_agent->own_message_queue);
        AR_HEAP_FREE(own_agent);
        return 0;
    }
    
    // Register the agent ID and track the agent object
    if (!ar_agent_registry_register_id(g_own_registry, new_id)) {
        ar_data_destroy(own_agent->own_memory);
        ar_list_destroy(own_agent->own_message_queue);
        AR_HEAP_FREE(own_agent);
        return 0;
    }
    
    if (!ar_agent_registry_track_agent(g_own_registry, new_id, own_agent)) {
        ar_agent_registry_unregister_id(g_own_registry, new_id);
        ar_data_destroy(own_agent->own_memory);
        ar_list_destroy(own_agent->own_message_queue);
        AR_HEAP_FREE(own_agent);
        return 0;
    }
    
    // Send wake message
    data_t *own_wake_msg = ar_data_create_string("__wake__");
    if (own_wake_msg) {
        ar_agent_send(new_id, own_wake_msg);
        // Note: The wake message will be processed when the system runs
    }
    
    return new_id;
}

bool ar_agent_destroy(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    // Find the agent in the registry
    agent_t *mut_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
    if (!mut_agent) {
        return false;
    }
    
    // Send sleep message before destruction
    data_t *own_sleep_msg = ar_data_create_string(g_sleep_message);
    if (own_sleep_msg) {
        bool sent = ar_list_add_last(mut_agent->own_message_queue, own_sleep_msg);
        if (!sent) {
            ar_data_destroy(own_sleep_msg);
            own_sleep_msg = NULL; // Mark as destroyed
        }
        else {
            // Note: The sleep message will be processed before the agent is destroyed
        }
    }
    
    // Untrack the agent from registry (gets the pointer back)
    agent_t *own_agent = (agent_t*)ar_agent_registry_untrack_agent(g_own_registry, agent_id);
    if (!own_agent) {
        return false;
    }
    
    // Unregister the agent ID
    ar_agent_registry_unregister_id(g_own_registry, agent_id);
    
    // Destroy memory if owned
    if (own_agent->own_memory) {
        ar_data_destroy(own_agent->own_memory);
        own_agent->own_memory = NULL; // Mark as destroyed
    }
    
    // We don't own the context, just clear the reference
    own_agent->ref_context = NULL;
    
    // Destroy any pending messages and the queue
    if (own_agent->own_message_queue) {
        // First, destroy any remaining messages in the queue
        data_t *own_msg = NULL;
        while ((own_msg = ar_list_remove_first(own_agent->own_message_queue)) != NULL) {
            ar_data_destroy(own_msg);
            own_msg = NULL; // Mark as destroyed
        }
        // Then destroy the queue itself
        ar_list_destroy(own_agent->own_message_queue);
        own_agent->own_message_queue = NULL; // Mark as destroyed
    }
    
    // Free the agent structure
    AR_HEAP_FREE(own_agent);
    
    // Debug output - agent destroyed
    printf("Agent %lld destroyed\n", (long long)agent_id);
    
    return true;
}

bool ar_agent_send(int64_t agent_id, data_t *own_message) {
    if (!g_is_initialized || !own_message || !g_own_registry) {
        // Destroy the message if we have one but not initialized
        if (own_message) {
            ar_data_destroy(own_message);
            own_message = NULL; // Mark as destroyed
        }
        return false;
    }
    
    // Find the agent in the registry
    agent_t *ref_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        // If we couldn't find the agent, destroy the message
        ar_data_destroy(own_message);
        own_message = NULL; // Mark as destroyed
        return false;
    }
    
    // Agent module takes ownership of the message and adds to the queue
    bool result = ar_list_add_last(ref_agent->own_message_queue, own_message);
    
    // If we couldn't add to the queue, destroy the message
    if (!result) {
        ar_data_destroy(own_message);
        own_message = NULL; // Mark as destroyed
    }
    
    return result;
}

bool ar_agent_exists(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    return ar_agent_registry_is_registered(g_own_registry, agent_id);
}

/* Accessor functions for opaque type */

const data_t* ar_agent_get_memory(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return NULL;
    }
    
    agent_t *ref_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        return NULL;
    }
    
    return ref_agent->own_memory;
}

data_t* ar_agent_get_mutable_memory(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return NULL;
    }
    
    agent_t *ref_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        return NULL;
    }
    
    return ref_agent->own_memory;
}

const data_t* ar_agent_get_context(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return NULL;
    }
    
    agent_t *ref_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        return NULL;
    }
    
    return ref_agent->ref_context;
}

bool ar_agent_is_active(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    agent_t *ref_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        return false;
    }
    
    return ref_agent->is_active;
}

bool ar_agent_get_method_info(int64_t agent_id, const char **out_method_name, const char **out_method_version) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    agent_t *ref_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
    if (!ref_agent || !ref_agent->ref_method) {
        return false;
    }
    
    if (out_method_name) {
        *out_method_name = ar_method_get_name(ref_agent->ref_method);
    }
    if (out_method_version) {
        *out_method_version = ar_method_get_version(ref_agent->ref_method);
    }
    return true;
}


const method_t* ar_agent_get_method(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return NULL;
    }
    
    agent_t *ref_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        return NULL;
    }
    
    return ref_agent->ref_method;
}

bool ar_agent_set_active(int64_t agent_id, bool is_active) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    agent_t *mut_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
    if (!mut_agent) {
        return false;
    }
    
    mut_agent->is_active = is_active;
    return true;
}


void ar_agent_reset_all(void) {
    if (!g_is_initialized || !g_own_registry) {
        return;
    }
    
    // Destroy all agents
    int64_t agent_id = ar_agent_registry_get_first(g_own_registry);
    while (agent_id != 0) {
        int64_t next_id = ar_agent_registry_get_next(g_own_registry, agent_id);
        ar_agent_destroy(agent_id);
        agent_id = next_id;
    }
    
    // Destroy the registry itself
    ar_agent_registry_destroy(g_own_registry);
    g_own_registry = NULL;
    g_is_initialized = false;
}

int64_t ar_agent_get_first_active(void) {
    if (!g_is_initialized) {
        ar_agent_init();
    }
    
    if (!g_own_registry) {
        return 0;
    }
    
    // For now, all registered agents are considered active
    return ar_agent_registry_get_first(g_own_registry);
}

int64_t ar_agent_get_next_active(int64_t current_id) {
    if (!g_is_initialized) {
        ar_agent_init();
    }
    
    if (!g_own_registry) {
        return 0;
    }
    
    // For now, all registered agents are considered active
    return ar_agent_registry_get_next(g_own_registry, current_id);
}

bool ar_agent_has_messages(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    agent_t *ref_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
    if (!ref_agent || !ref_agent->own_message_queue) {
        return false;
    }
    
    return ar_list_count(ref_agent->own_message_queue) > 0;
}

data_t* ar_agent_get_message(int64_t agent_id) {
    if (!g_is_initialized || !g_own_registry) {
        return NULL;
    }
    
    agent_t *ref_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
    if (!ref_agent || !ref_agent->own_message_queue) {
        return NULL;
    }
    
    return ar_list_remove_first(ref_agent->own_message_queue);
}

int ar_agent_count_active(void) {
    if (!g_is_initialized || !g_own_registry) {
        return 0;
    }
    
    // For now, all registered agents are considered active
    return ar_agent_registry_count(g_own_registry);
}

list_t* ar_agent_get_active_list(void) {
    if (!g_is_initialized || !g_own_registry) {
        return NULL;
    }
    
    list_t *own_list = ar_list_create();
    if (!own_list) {
        return NULL;
    }
    
    int64_t agent_id = ar_agent_registry_get_first(g_own_registry);
    while (agent_id != 0) {
        data_t *own_id_data = ar_data_create_integer((int)agent_id);
        if (own_id_data) {
            ar_list_add_last(own_list, own_id_data);
        }
        agent_id = ar_agent_registry_get_next(g_own_registry, agent_id);
    }
    
    return own_list;
}

int ar_agent_count_by_method(const method_t *ref_method) {
    if (!g_is_initialized || !g_own_registry || !ref_method) {
        return 0;
    }
    
    int count = 0;
    int64_t agent_id = ar_agent_registry_get_first(g_own_registry);
    while (agent_id != 0) {
        agent_t *ref_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
        if (ref_agent && ref_agent->ref_method == ref_method) {
            count++;
        }
        agent_id = ar_agent_registry_get_next(g_own_registry, agent_id);
    }
    
    return count;
}

int ar_agent_update_method(const method_t *ref_old_method, const method_t *ref_new_method, bool send_sleep_wake) {
    if (!g_is_initialized || !g_own_registry || !ref_old_method || !ref_new_method) {
        return 0;
    }
    
    int count = 0;
    int64_t agent_id = ar_agent_registry_get_first(g_own_registry);
    while (agent_id != 0) {
        agent_t *mut_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
        if (mut_agent && mut_agent->ref_method == ref_old_method) {
            if (send_sleep_wake) {
                // Send sleep message before update
                data_t *own_sleep_msg = ar_data_create_string(g_sleep_message);
                if (own_sleep_msg) {
                    ar_list_add_last(mut_agent->own_message_queue, own_sleep_msg);
                }
            }
            
            // Update the method
            mut_agent->ref_method = ref_new_method;
            
            if (send_sleep_wake) {
                // Send wake message after update
                data_t *own_wake_msg = ar_data_create_string("__wake__");
                if (own_wake_msg) {
                    ar_list_add_last(mut_agent->own_message_queue, own_wake_msg);
                }
            }
            
            count++;
        }
        agent_id = ar_agent_registry_get_next(g_own_registry, agent_id);
    }
    
    return count;
}

bool ar_agent_get_persistence_info(int64_t agent_id, const method_t **out_method, const data_t **out_memory) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    agent_t *ref_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, agent_id);
    if (!ref_agent) {
        return false;
    }
    
    if (out_method) {
        *out_method = ref_agent->ref_method;
    }
    if (out_memory) {
        *out_memory = ref_agent->own_memory;
    }
    
    return true;
}

bool ar_agent_set_id(int64_t old_id, int64_t new_id) {
    if (!g_is_initialized || !g_own_registry) {
        return false;
    }
    
    agent_t *mut_agent = (agent_t*)ar_agent_registry_find_agent(g_own_registry, old_id);
    if (!mut_agent) {
        return false;
    }
    
    // Update the agent's ID
    mut_agent->id = new_id;
    
    // Update in registry: untrack from old ID, register new ID, track with new ID
    ar_agent_registry_untrack_agent(g_own_registry, old_id);
    ar_agent_registry_unregister_id(g_own_registry, old_id);
    
    if (!ar_agent_registry_register_id(g_own_registry, new_id)) {
        // Restore old ID if registration fails
        mut_agent->id = old_id;
        ar_agent_registry_register_id(g_own_registry, old_id);
        ar_agent_registry_track_agent(g_own_registry, old_id, mut_agent);
        return false;
    }
    
    if (!ar_agent_registry_track_agent(g_own_registry, new_id, mut_agent)) {
        // Restore old ID if tracking fails
        mut_agent->id = old_id;
        ar_agent_registry_unregister_id(g_own_registry, new_id);
        ar_agent_registry_register_id(g_own_registry, old_id);
        ar_agent_registry_track_agent(g_own_registry, old_id, mut_agent);
        return false;
    }
    
    return true;
}

