/* Agerun Runtime System Implementation */
#include "ar_system.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_data.h"
#include "ar_agent.h"
#include "ar_agency.h"
#include "ar_list.h"
#include "ar_map.h"
#include "ar_heap.h"
#include "ar_interpreter.h"
#include "ar_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>

/* Constants are now defined in their respective module header files:
 * - MAX_METHODS, MAX_VERSIONS_PER_METHOD, MAX_METHOD_NAME_LENGTH, MAX_INSTRUCTIONS_LENGTH in ar_methodology.h
 * - Lists used for message queues provide dynamic sizing
 * - MAX_AGENTS, MAX_METHOD_NAME_LENGTH in ar_agent.h
 */

/* Static variables for commonly used messages */
static char g_wake_message[] = "__wake__";

/* Memory Map structure is now defined in ar_map.h */

/* Message Queue functionality is now provided by the list module */

/* Method Definition is now in ar_method.h/c */

/* Agent Definition is now in ar_agent.h */

/* System structure */
struct ar_system_s {
    bool is_initialized;
    ar_agency_t *ref_agency;         // Borrowed reference (NULL = use global agency)
    ar_interpreter_t *own_interpreter; // Always owned by the system
    ar_log_t *own_log;                // Always owned by the system
};

/* Global State */
static ar_system_t *g_system = NULL;  // Single global instance

/* Implementation */
int64_t ar_system__init(const char *ref_method_name, const char *ref_version) {
    if (g_system) {
        printf("Agerun already initialized\n");
        return 0;
    }
    
    // Create the global system instance using NULL for global agency
    g_system = ar_system__create(NULL);
    if (!g_system) {
        printf("Error: Failed to create system instance\n");
        return 0;
    }
    
    // Set global agency as initialized
    ar_agency__set_initialized(true);
    
    // Delegate to instance-based init
    return ar_system__init_with_instance(g_system, ref_method_name, ref_version);
}

void ar_system__shutdown(void) {
    if (!g_system) {
        return;
    }
    
    // Delegate to instance-based shutdown
    ar_system__shutdown_with_instance(g_system);
    
    // Destroy the global system instance
    ar_system__destroy(g_system);
    g_system = NULL;
    
    // Mark global agency as uninitialized
    ar_agency__set_initialized(false);
}

bool ar_system__process_next_message(void) {
    if (!g_system) {
        return false;
    }
    
    // Delegate to instance-based implementation
    return ar_system__process_next_message_with_instance(g_system);
}

int ar_system__process_all_messages(void) {
    if (!g_system) {
        return 0;
    }
    
    // Delegate to instance-based implementation
    return ar_system__process_all_messages_with_instance(g_system);
}

/* Instance-based API implementation */
ar_system_t* ar_system__create(ar_agency_t *ref_agency) {
    ar_system_t *own_system = AR__HEAP__MALLOC(sizeof(ar_system_t), "system");
    if (!own_system) {
        return NULL;
    }
    
    // Initialize fields
    own_system->is_initialized = false;
    own_system->ref_agency = ref_agency;
    
    // Create owned resources
    own_system->own_log = ar_log__create();
    if (!own_system->own_log) {
        AR__HEAP__FREE(own_system);
        return NULL;
    }
    
    own_system->own_interpreter = ar_interpreter__create(own_system->own_log);
    if (!own_system->own_interpreter) {
        ar_log__destroy(own_system->own_log);
        AR__HEAP__FREE(own_system);
        return NULL;
    }
    
    return own_system;
}

void ar_system__destroy(ar_system_t *own_system) {
    if (!own_system) {
        return;
    }
    
    // Destroy owned resources
    if (own_system->own_interpreter) {
        ar_interpreter__destroy(own_system->own_interpreter);
    }
    
    if (own_system->own_log) {
        ar_log__destroy(own_system->own_log);
    }
    
    // Clear global reference if this is the global instance
    if (own_system == g_system) {
        g_system = NULL;
    }
    
    AR__HEAP__FREE(own_system);
}

int64_t ar_system__init_with_instance(ar_system_t *mut_system, const char *ref_method_name, const char *ref_version) {
    if (!mut_system) {
        return 0;
    }
    
    if (mut_system->is_initialized) {
        printf("System instance already initialized\n");
        return 0;
    }
    
    mut_system->is_initialized = true;
    
    // If we have an agency instance, initialize it
    if (mut_system->ref_agency) {
        ar_methodology_t *ref_methodology = ar_agency__get_methodology(mut_system->ref_agency);
        
        // Load methods from file if available
        if (ref_methodology) {
            if (!ar_methodology__load_methods_with_instance(ref_methodology, NULL)) {
                printf("Warning: Could not load methods from file\n");
            }
        } else {
            // No methodology in agency means we're using global methodology
            if (!ar_methodology__load_methods()) {
                printf("Warning: Could not load methods from file\n");
            }
        }
        
        // Load agents from file if available
        if (!ar_agency__load_agents_with_instance(mut_system->ref_agency, NULL)) {
            printf("Warning: Could not load agents from file\n");
        }
    } else {
        // Use global methodology and agency
        if (!ar_methodology__load_methods()) {
            printf("Warning: Could not load methods from file\n");
        }
        
        if (!ar_agency__load_agents()) {
            printf("Warning: Could not load agents from file\n");
        }
    }
    
    // Create initial agent if ref_method_name is provided
    if (ref_method_name != NULL) {
        int64_t initial_agent;
        ar_data_t *own_wake_data;
        
        if (mut_system->ref_agency) {
            // Use instance-based agency
            initial_agent = ar_agency__create_agent_with_instance(mut_system->ref_agency, 
                                                                 ref_method_name, 
                                                                 ref_version, 
                                                                 NULL);
            
            if (initial_agent != 0) {
                // Send wake message to initial agent
                own_wake_data = ar_data__create_string(g_wake_message);
                if (own_wake_data) {
                    ar_agency__send_to_agent_with_instance(mut_system->ref_agency, 
                                                          initial_agent, 
                                                          own_wake_data);
                    
                    // Process the wake message
                    ar_system__process_next_message_with_instance(mut_system);
                }
            }
        } else {
            // Use global agency
            initial_agent = ar_agency__create_agent(ref_method_name, ref_version, NULL);
            
            if (initial_agent != 0) {
                // Send wake message to initial agent
                own_wake_data = ar_data__create_string(g_wake_message);
                if (own_wake_data) {
                    ar_agency__send_to_agent(initial_agent, own_wake_data);
                    
                    // Process the wake message
                    ar_system__process_next_message_with_instance(mut_system);
                }
            }
        }
        return initial_agent;
    }
    
    return 0;
}

void ar_system__shutdown_with_instance(ar_system_t *mut_system) {
    if (!mut_system || !mut_system->is_initialized) {
        return;
    }
    
    // If we have an agency instance, shut it down properly
    if (mut_system->ref_agency) {
        ar_methodology_t *ref_methodology = ar_agency__get_methodology(mut_system->ref_agency);
        
        // Save methods to file
        if (ref_methodology) {
            ar_methodology__save_methods_with_instance(ref_methodology, NULL);
        } else {
            // No methodology in agency means we're using global methodology
            ar_methodology__save_methods();
        }
        
        // Save persistent agents to file
        ar_agency__save_agents_with_instance(mut_system->ref_agency, NULL);
        
        // Reset the agency to clean up all agents
        ar_agency__reset_with_instance(mut_system->ref_agency);
        
        // Clean up methodology if we were using the global one
        if (!ref_methodology) {
            ar_methodology__cleanup();
        }
    } else {
        // Use global methodology and agency
        ar_methodology__save_methods();
        ar_agency__save_agents();
        ar_agency__reset();
        ar_methodology__cleanup();
    }
    
    mut_system->is_initialized = false;
}

bool ar_system__process_next_message_with_instance(ar_system_t *mut_system) {
    if (!mut_system || !mut_system->is_initialized) {
        return false;
    }
    
    // Find an agent with a non-empty message queue
    int64_t agent_id;
    ar_data_t *own_message = NULL;
    
    if (mut_system->ref_agency) {
        // Use instance-based agency
        agent_id = ar_agency__get_first_agent_with_instance(mut_system->ref_agency);
        while (agent_id != 0) {
            if (ar_agency__agent_has_messages_with_instance(mut_system->ref_agency, agent_id)) {
                own_message = ar_agency__get_agent_message_with_instance(mut_system->ref_agency, agent_id);
                if (own_message) {
                    break;
                }
            }
            agent_id = ar_agency__get_next_agent_with_instance(mut_system->ref_agency, agent_id);
        }
    } else {
        // Use global agency
        agent_id = ar_agency__get_first_agent();
        while (agent_id != 0) {
            if (ar_agency__agent_has_messages(agent_id)) {
                own_message = ar_agency__get_agent_message(agent_id);
                if (own_message) {
                    break;
                }
            }
            agent_id = ar_agency__get_next_agent(agent_id);
        }
    }
    
    if (own_message) {
        // Print message based on its type
        printf("Agent %" PRId64 " received message: ", agent_id);
        ar_data_type_t msg_type = ar_data__get_type(own_message);
        if (msg_type == AR_DATA_TYPE__STRING) {
            printf("%s\n", ar_data__get_string(own_message));
        } else if (msg_type == AR_DATA_TYPE__INTEGER) {
            printf("%d\n", ar_data__get_integer(own_message));
        } else if (msg_type == AR_DATA_TYPE__DOUBLE) {
            printf("%f\n", ar_data__get_double(own_message));
        } else if (msg_type == AR_DATA_TYPE__LIST || msg_type == AR_DATA_TYPE__MAP) {
            printf("[complex data]\n");
        }
        
        ar_interpreter__execute_method(mut_system->own_interpreter, agent_id, own_message);
        
        // Free the message as it's now been processed
        ar_data__destroy_if_owned(own_message, &mut_system->is_initialized);
        own_message = NULL; // Mark as freed
        return true;
    }
    
    return false; // No messages to process
}

int ar_system__process_all_messages_with_instance(ar_system_t *mut_system) {
    int count = 0;
    
    while (ar_system__process_next_message_with_instance(mut_system)) {
        count++;
    }
    
    return count;
}

