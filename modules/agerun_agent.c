/* Agerun Agent Implementation */
#include "agerun_agent.h"
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

/* Global State - moved from agency module */
static agent_t g_own_agents[MAX_AGENTS]; // Owned by the agent module
static int64_t g_next_agent_id = 1;
static bool g_is_initialized = false;

/* Static variables for commonly used messages */
static const char g_sleep_message[] = "__sleep__";

/* Static initialization */
static void ar_agent_init(void) {
    if (!g_is_initialized) {
        for (int i = 0; i < MAX_AGENTS; i++) {
            g_own_agents[i].is_active = false;
            g_own_agents[i].ref_method = NULL;
            g_own_agents[i].own_memory = NULL;
            g_own_agents[i].own_message_queue = NULL;
            g_own_agents[i].ref_context = NULL;
        }
        g_is_initialized = true;
    }
}

/* Internal accessor functions */
agent_t* ar_agent_get_agents_internal(void) {
    if (!g_is_initialized) {
        ar_agent_init();
    }
    return g_own_agents;
}

int64_t ar_agent_get_next_id_internal(void) {
    return g_next_agent_id;
}

void ar_agent_set_next_id_internal(int64_t id) {
    g_next_agent_id = id;
}

/* Implementation */
int64_t ar_agent_create(const char *ref_method_name, const char *ref_version, const data_t *ref_context) {
    if (!g_is_initialized) {
        ar_agent_init();
    }
    
    if (!ref_method_name) {
        // No need to free ref_context as we don't own it
        return 0;
    }
    
    // Find free slot for new agent
    int agent_idx = -1;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (!g_own_agents[i].is_active) {
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
    int64_t new_id = g_next_agent_id;
    g_own_agents[agent_idx].id = new_id;
    g_own_agents[agent_idx].is_active = true;
    g_own_agents[agent_idx].ref_method = ref_method;  // Just store reference
    g_next_agent_id = new_id + 1;  // Increment for next agent
    
    g_own_agents[agent_idx].own_message_queue = ar_list_create();
    g_own_agents[agent_idx].own_memory = ar_data_create_map();
    g_own_agents[agent_idx].ref_context = ref_context;  // Store reference, we don't own it
    
    // Send wake message
    data_t *own_wake_msg = ar_data_create_string("__wake__");
    if (own_wake_msg) {
        ar_agent_send(new_id, own_wake_msg);
        // Note: The wake message will be processed when the system runs
    }
    
    return new_id;
}

bool ar_agent_destroy(int64_t agent_id) {
    if (!g_is_initialized) {
        return false;
    }
    
    // Find the agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == agent_id) {
            // Send sleep message before destruction
            data_t *own_sleep_msg = ar_data_create_string(g_sleep_message);
            if (own_sleep_msg) {
                bool sent = ar_list_add_last(g_own_agents[i].own_message_queue, own_sleep_msg);
                if (!sent) {
                    ar_data_destroy(own_sleep_msg);
                    own_sleep_msg = NULL; // Mark as destroyed
                }
                else {
                    // Note: The sleep message will be processed before the agent is destroyed
                }
            }
            
            // Destroy memory if owned
            if (g_own_agents[i].own_memory) {
                ar_data_destroy(g_own_agents[i].own_memory);
                g_own_agents[i].own_memory = NULL; // Mark as destroyed
            }
            
            // We don't own the context, just clear the reference
            g_own_agents[i].ref_context = NULL;
            
            // Destroy any pending messages and the queue
            if (g_own_agents[i].own_message_queue) {
                // First, destroy any remaining messages in the queue
                data_t *own_msg = NULL;
                while ((own_msg = ar_list_remove_first(g_own_agents[i].own_message_queue)) != NULL) {
                    ar_data_destroy(own_msg);
                    own_msg = NULL; // Mark as destroyed
                }
                // Then destroy the queue itself
                ar_list_destroy(g_own_agents[i].own_message_queue);
                g_own_agents[i].own_message_queue = NULL; // Mark as destroyed
            }
            
            // Mark agent as inactive and clear ID
            g_own_agents[i].is_active = false;
            g_own_agents[i].id = 0;
            
            // Debug output - agent destroyed
            printf("Agent %lld destroyed\n", (long long)agent_id);
            
            return true;
        }
    }
    
    return false;
}

bool ar_agent_send(int64_t agent_id, data_t *own_message) {
    if (!g_is_initialized || !own_message) {
        // Destroy the message if we have one but not initialized
        if (own_message) {
            ar_data_destroy(own_message);
            own_message = NULL; // Mark as destroyed
        }
        return false;
    }
    
    // Find the agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == agent_id) {
            // Agent module takes ownership of the message and adds to the queue
            bool result = ar_list_add_last(g_own_agents[i].own_message_queue, own_message);
            
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
    // Direct access to local array
    
    if (!g_is_initialized) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == agent_id) {
            return true;
        }
    }
    
    return false;
}

/* Accessor functions for opaque type */

const data_t* ar_agent_get_memory(int64_t agent_id) {
    // Direct access to local array
    
    if (!g_is_initialized) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == agent_id) {
            return g_own_agents[i].own_memory;
        }
    }
    
    return NULL;
}

data_t* ar_agent_get_mutable_memory(int64_t agent_id) {
    if (!g_is_initialized) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == agent_id) {
            return g_own_agents[i].own_memory;
        }
    }
    
    return NULL;
}

const data_t* ar_agent_get_context(int64_t agent_id) {
    // Direct access to local array
    
    if (!g_is_initialized) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == agent_id) {
            return g_own_agents[i].ref_context;
        }
    }
    
    return NULL;
}

bool ar_agent_is_active(int64_t agent_id) {
    // Direct access to local array
    
    if (!g_is_initialized) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].id == agent_id) {
            return g_own_agents[i].is_active;
        }
    }
    
    return false;
}

bool ar_agent_get_method_info(int64_t agent_id, const char **out_method_name, const char **out_method_version) {
    // Direct access to local array
    
    if (!g_is_initialized) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == agent_id && g_own_agents[i].ref_method != NULL) {
            if (out_method_name) {
                *out_method_name = ar_method_get_name(g_own_agents[i].ref_method);
            }
            if (out_method_version) {
                *out_method_version = ar_method_get_version(g_own_agents[i].ref_method);
            }
            return true;
        }
    }
    
    return false;
}

const method_t* ar_agent_get_method(int64_t agent_id) {
    // Direct access to local array
    
    if (!g_is_initialized) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == agent_id) {
            return g_own_agents[i].ref_method;
        }
    }
    
    return NULL;
}

bool ar_agent_set_active(int64_t agent_id, bool is_active) {
    // Direct access to local array
    
    if (!g_is_initialized) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].id == agent_id) {
            g_own_agents[i].is_active = is_active;
            return true;
        }
    }
    
    return false;
}

agent_t* ar_agent_get_internal(int64_t agent_id) {
    // Direct access to local array
    
    if (!g_is_initialized) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == agent_id) {
            return &g_own_agents[i];
        }
    }
    
    return NULL;
}

void ar_agent_reset_all(void) {
    // Reset all agents to inactive
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active) {
            // Free memory data if it exists
            if (g_own_agents[i].own_memory) {
                ar_data_destroy(g_own_agents[i].own_memory);
                g_own_agents[i].own_memory = NULL; // Mark as destroyed
            }
            
            // Clear context reference (we don't own it)
            g_own_agents[i].ref_context = NULL;
            
            // Free message queue if it exists
            if (g_own_agents[i].own_message_queue) {
                // First destroy any remaining messages in the queue
                data_t *own_msg = NULL;
                while ((own_msg = ar_list_remove_first(g_own_agents[i].own_message_queue)) != NULL) {
                    ar_data_destroy(own_msg);
                    own_msg = NULL; // Mark as destroyed
                }
                // Then destroy the queue itself
                ar_list_destroy(g_own_agents[i].own_message_queue);
                g_own_agents[i].own_message_queue = NULL; // Mark as destroyed
            }
        }
        g_own_agents[i].is_active = false;
    }
    
    // Reset next_agent_id
    g_next_agent_id = 1;
}

int64_t ar_agent_get_first_active(void) {
    if (!g_is_initialized) {
        ar_agent_init();
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active) {
            return g_own_agents[i].id;
        }
    }
    
    return 0; // No active agents
}

int64_t ar_agent_get_next_active(int64_t current_id) {
    if (!g_is_initialized) {
        ar_agent_init();
    }
    
    bool found_current = false;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (found_current && g_own_agents[i].is_active) {
            return g_own_agents[i].id;
        }
        if (g_own_agents[i].id == current_id) {
            found_current = true;
        }
    }
    
    return 0; // No more active agents
}

bool ar_agent_has_messages(int64_t agent_id) {
    if (!g_is_initialized) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == agent_id) {
            return g_own_agents[i].own_message_queue && !ar_list_empty(g_own_agents[i].own_message_queue);
        }
    }
    
    return false;
}

data_t* ar_agent_get_message(int64_t agent_id) {
    if (!g_is_initialized) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == agent_id && g_own_agents[i].own_message_queue) {
            return ar_list_remove_first(g_own_agents[i].own_message_queue);
        }
    }
    
    return NULL;
}

int ar_agent_count_active(void) {
    if (!g_is_initialized) {
        ar_agent_init();
    }
    
    int count = 0;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active) {
            count++;
        }
    }
    
    return count;
}

int ar_agent_iterate_active(ar_agent_visitor_fn visitor, void *user_data) {
    if (!g_is_initialized || !visitor) {
        return 0;
    }
    
    int visited = 0;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active) {
            if (!visitor(g_own_agents[i].id, g_own_agents[i].ref_method, 
                        g_own_agents[i].own_memory, user_data)) {
                break;
            }
            visited++;
        }
    }
    
    return visited;
}

int ar_agent_count_by_method(const method_t *ref_method) {
    if (!ref_method) {
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].ref_method == ref_method) {
            count++;
        }
    }
    
    return count;
}

int ar_agent_update_method(const method_t *ref_old_method, const method_t *ref_new_method, bool send_sleep_wake) {
    if (!ref_old_method || !ref_new_method) {
        return 0;
    }
    
    int update_count = 0;
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].ref_method == ref_old_method) {
            int64_t agent_id = g_own_agents[i].id;
            
            if (send_sleep_wake) {
                // Send sleep message
                data_t *own_sleep_msg = ar_data_create_string("__sleep__");
                if (own_sleep_msg) {
                    ar_agent_send(agent_id, own_sleep_msg);
                }
            }
            
            // Update the method reference
            g_own_agents[i].ref_method = ref_new_method;
            
            if (send_sleep_wake) {
                // Send wake message
                data_t *own_wake_msg = ar_data_create_string("__wake__");
                if (own_wake_msg) {
                    ar_agent_send(agent_id, own_wake_msg);
                }
            }
            
            update_count++;
        }
    }
    
    return update_count;
}

bool ar_agent_get_persistence_info(int64_t agent_id, const method_t **out_method, const data_t **out_memory) {
    if (!g_is_initialized) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == agent_id) {
            if (out_method) {
                *out_method = g_own_agents[i].ref_method;
            }
            if (out_memory) {
                *out_memory = g_own_agents[i].own_memory;
            }
            return true;
        }
    }
    
    return false;
}

bool ar_agent_set_id(int64_t old_id, int64_t new_id) {
    if (!g_is_initialized) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].id == old_id) {
            g_own_agents[i].id = new_id;
            return true;
        }
    }
    
    return false;
}
