/* Agerun Runtime System Implementation */
#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
#include "agerun_data.h"
#include "agerun_agent.h"
#include "agerun_agency.h"
#include "agerun_list.h"
#include "agerun_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/* Constants are now defined in their respective module header files:
 * - MAX_METHODS, MAX_VERSIONS_PER_METHOD, MAX_METHOD_NAME_LENGTH, MAX_INSTRUCTIONS_LENGTH in agerun_methodology.h
 * - Lists used for message queues provide dynamic sizing
 * - MAX_AGENTS, MAX_METHOD_NAME_LENGTH in agerun_agent.h
 */

/* Static variables for commonly used messages */
static char g_wake_message[] = "__wake__";

/* Memory Map structure is now defined in agerun_map.h */

/* Message Queue is now defined in agerun_queue.h */

/* Method Definition is now in agerun_method.h/c */

/* Agent Definition is now in agerun_agent.h */

/* Global State */
static bool is_initialized = false;

/* Implementation */
agent_id_t ar_system_init(const char *method_name, version_t version) {
    if (is_initialized) {
        printf("Agerun already initialized\n");
        return 0;
    }
    
    is_initialized = true;
    ar_agency_set_initialized(true);
    
    // Load methods from file if available
    if (!ar_methodology_load_methods()) {
        printf("Warning: Could not load methods from file\n");
    }
    
    // Load agents from file if available
    if (!ar_agency_load_agents()) {
        printf("Warning: Could not load agents from file\n");
    }
    
    // Create initial agent if method_name is provided
    if (method_name != NULL) {
        // Create initial agent with NULL context
        agent_id_t initial_agent = ar_agent_create(method_name, version, NULL);
        if (initial_agent != 0) {
            // Send wake message to initial agent
            ar_agent_send(initial_agent, g_wake_message);
        }
        return initial_agent;
    }
    
    return 0;
}

void ar_system_shutdown(void) {
    if (!is_initialized) {
        return;
    }
    
    // Save methods to file
    ar_methodology_save_methods();
    
    // Save persistent agents to file
    ar_agency_save_agents();
    
    // The memory cleanup is now responsibility of the agency module
    
    is_initialized = false;
    ar_agency_reset();
    ar_agency_set_initialized(false);
}



bool ar_system_process_next_message(void) {
    if (!is_initialized) {
        return false;
    }
    
    // Find an agent with a non-empty message queue
    agent_t* agents = ar_agency_get_agents();
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && !ar_list_empty(agents[i].message_queue)) {
            // Process one message
            char *message = ar_list_remove_first(agents[i].message_queue);
            if (message) {
                // Use the interpret_method function from agerun_agent
                // Since that's now private, we need to call the method directly
                method_t *method = ar_methodology_get_method(agents[i].method_name, agents[i].method_version);
                if (method) {
                    printf("Agent %lld received message: %s\n", agents[i].id, message);
                    ar_method_run(&agents[i], message, method->instructions);
                    return true;
                }
            }
        }
    }
    
    return false; // No messages to process
}

int ar_system_process_all_messages(void) {
    int count = 0;
    
    while (ar_system_process_next_message()) {
        count++;
    }
    
    return count;
}

/* This function is now implemented in agerun_agent.c */

/* The functions ar_count_agents, ar_save_agents, ar_load_agents, ar_methodology_save_methods, 
 * ar_methodology_load_methods, and ar_memory_set have been moved to other modules */

// Memory functions are now defined in agerun_map.c

// This function has been moved to agerun_value.c

/* Queue functions are now defined in agerun_queue.c */



/* Memory functions are now defined in agerun_map.c */

// Memory functions are now defined in agerun_map.c
