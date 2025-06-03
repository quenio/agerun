/* Agerun Agent Implementation */
#include "agerun_agent.h"
#include "agerun_agency.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
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

/* Static variables for commonly used messages */
static const char g_sleep_message[] = "__sleep__";

/* Implementation */
int64_t ar_agent_create(const char *ref_method_name, const char *ref_version, const data_t *ref_context) {
    agent_t *mut_agents = ar_agency_get_agents();
    
    if (mut_agents == NULL || !ref_method_name) {
        // No need to free ref_context as we don't own it
        return 0;
    }
    
    // Find free slot for new agent
    int agent_idx = -1;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (!mut_agents[i].is_active) {
            agent_idx = i;
            break;
        }
    }
    
    // No space available
    if (agent_idx == -1) {
        // No need to free ref_context as we don't own it
        return 0;
    }
    
    // Lookup method using methodology
    const method_t *ref_method = ar_methodology_get_method(ref_method_name, ref_version);
    if (!ref_method) {
        // No need to free ref_context as we don't own it
        return 0;  // Method not found
    }
    
    // Initialize agent
    int64_t new_id = ar_agency_get_next_id();
    mut_agents[agent_idx].id = new_id;
    mut_agents[agent_idx].is_active = true;
    mut_agents[agent_idx].ref_method = ref_method;  // Just store reference
    ar_agency_set_next_id(new_id + 1);  // Increment for next agent
    
    mut_agents[agent_idx].own_message_queue = ar_list_create();
    mut_agents[agent_idx].own_memory = ar_data_create_map();
    mut_agents[agent_idx].ref_context = ref_context;  // Store reference, we don't own it
    
    // Send wake message
    data_t *own_wake_msg = ar_data_create_string("__wake__");
    if (own_wake_msg) {
        ar_agent_send(new_id, own_wake_msg);
        // Note: The wake message will be processed when the system runs
    }
    
    return new_id;
}

bool ar_agent_destroy(int64_t agent_id) {
    agent_t *mut_agents = ar_agency_get_agents();
    
    if (mut_agents == NULL) {
        return false;
    }
    
    // Find the agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (mut_agents[i].is_active && mut_agents[i].id == agent_id) {
            // Send sleep message before destruction
            data_t *own_sleep_msg = ar_data_create_string(g_sleep_message);
            if (own_sleep_msg) {
                bool sent = ar_list_add_last(mut_agents[i].own_message_queue, own_sleep_msg);
                if (!sent) {
                    ar_data_destroy(own_sleep_msg);
                    own_sleep_msg = NULL; // Mark as destroyed
                }
                else {
                    // Note: The sleep message will be processed before the agent is destroyed
                }
            }
            
            // Destroy memory if owned
            if (mut_agents[i].own_memory) {
                ar_data_destroy(mut_agents[i].own_memory);
                mut_agents[i].own_memory = NULL; // Mark as destroyed
            }
            
            // We don't own the context, just clear the reference
            mut_agents[i].ref_context = NULL;
            
            // Destroy any pending messages and the queue
            if (mut_agents[i].own_message_queue) {
                // First, destroy any remaining messages in the queue
                data_t *own_msg = NULL;
                while ((own_msg = ar_list_remove_first(mut_agents[i].own_message_queue)) != NULL) {
                    ar_data_destroy(own_msg);
                    own_msg = NULL; // Mark as destroyed
                }
                // Then destroy the queue itself
                ar_list_destroy(mut_agents[i].own_message_queue);
                mut_agents[i].own_message_queue = NULL; // Mark as destroyed
            }
            
            // Mark agent as inactive and clear ID
            mut_agents[i].is_active = false;
            mut_agents[i].id = 0;
            
            // Debug output - agent destroyed
            printf("Agent %lld destroyed\n", (long long)agent_id);
            
            return true;
        }
    }
    
    return false;
}

bool ar_agent_send(int64_t agent_id, data_t *own_message) {
    agent_t *mut_agents = ar_agency_get_agents();
    
    if (mut_agents == NULL || !own_message) {
        // Destroy the message if we have one but no agents
        if (own_message) {
            ar_data_destroy(own_message);
            own_message = NULL; // Mark as destroyed
        }
        return false;
    }
    
    // Find the agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (mut_agents[i].is_active && mut_agents[i].id == agent_id) {
            // Agent module takes ownership of the message and adds to the queue
            bool result = ar_list_add_last(mut_agents[i].own_message_queue, own_message);
            
            // If we couldn't add to the queue, destroy the message
            if (!result) {
                ar_data_destroy(own_message);
                own_message = NULL; // Mark as destroyed
            }
            
            return result;
        }
    }
    
    // If we couldn't find the agent, destroy the message
    ar_data_destroy(own_message);
    own_message = NULL; // Mark as destroyed
    return false;
}

bool ar_agent_exists(int64_t agent_id) {
    const agent_t *ref_agents = ar_agency_get_agents();
    
    if (ref_agents == NULL) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (ref_agents[i].is_active && ref_agents[i].id == agent_id) {
            return true;
        }
    }
    
    return false;
}

/* Accessor functions for opaque type */

const data_t* ar_agent_get_memory(int64_t agent_id) {
    const agent_t *ref_agents = ar_agency_get_agents();
    
    if (ref_agents == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (ref_agents[i].is_active && ref_agents[i].id == agent_id) {
            return ref_agents[i].own_memory;
        }
    }
    
    return NULL;
}

data_t* ar_agent_get_mutable_memory(int64_t agent_id) {
    agent_t *mut_agents = ar_agency_get_agents();
    
    if (mut_agents == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (mut_agents[i].is_active && mut_agents[i].id == agent_id) {
            return mut_agents[i].own_memory;
        }
    }
    
    return NULL;
}

const data_t* ar_agent_get_context(int64_t agent_id) {
    const agent_t *ref_agents = ar_agency_get_agents();
    
    if (ref_agents == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (ref_agents[i].is_active && ref_agents[i].id == agent_id) {
            return ref_agents[i].ref_context;
        }
    }
    
    return NULL;
}

bool ar_agent_is_active(int64_t agent_id) {
    const agent_t *ref_agents = ar_agency_get_agents();
    
    if (ref_agents == NULL) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (ref_agents[i].id == agent_id) {
            return ref_agents[i].is_active;
        }
    }
    
    return false;
}

bool ar_agent_get_method_info(int64_t agent_id, const char **out_method_name, const char **out_method_version) {
    const agent_t *ref_agents = ar_agency_get_agents();
    
    if (ref_agents == NULL) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (ref_agents[i].is_active && ref_agents[i].id == agent_id && ref_agents[i].ref_method != NULL) {
            if (out_method_name) {
                *out_method_name = ar_method_get_name(ref_agents[i].ref_method);
            }
            if (out_method_version) {
                *out_method_version = ar_method_get_version(ref_agents[i].ref_method);
            }
            return true;
        }
    }
    
    return false;
}

const method_t* ar_agent_get_method(int64_t agent_id) {
    const agent_t *ref_agents = ar_agency_get_agents();
    
    if (ref_agents == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (ref_agents[i].is_active && ref_agents[i].id == agent_id) {
            return ref_agents[i].ref_method;
        }
    }
    
    return NULL;
}

bool ar_agent_set_active(int64_t agent_id, bool is_active) {
    agent_t *mut_agents = ar_agency_get_agents();
    
    if (mut_agents == NULL) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (mut_agents[i].id == agent_id) {
            mut_agents[i].is_active = is_active;
            return true;
        }
    }
    
    return false;
}

agent_t* ar_agent_get_internal(int64_t agent_id) {
    agent_t *mut_agents = ar_agency_get_agents();
    
    if (mut_agents == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (mut_agents[i].is_active && mut_agents[i].id == agent_id) {
            return &mut_agents[i];
        }
    }
    
    return NULL;
}
