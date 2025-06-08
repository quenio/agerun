/* Agerun Runtime System Implementation */
#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
#include "agerun_data.h"
#include "agerun_agent.h"
#include "agerun_agency.h"
#include "agerun_list.h"
#include "agerun_map.h"
#include "agerun_heap.h"

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

/* Message Queue functionality is now provided by the list module */

/* Method Definition is now in agerun_method.h/c */

/* Agent Definition is now in agerun_agent.h */

/* Global State */
static bool is_initialized = false;

/* Implementation */
int64_t ar__system__init(const char *ref_method_name, const char *ref_version) {
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
    
    // Create initial agent if ref_method_name is provided
    if (ref_method_name != NULL) {
        // Create initial agent with NULL context
        int64_t initial_agent = ar_agency_create_agent(ref_method_name, ref_version, NULL);
        if (initial_agent != 0) {
            // Send wake message to initial agent
            data_t *own_wake_data = ar_data_create_string(g_wake_message);
            if (own_wake_data) {
                ar_agency_send_to_agent(initial_agent, own_wake_data);
                // Ownership transferred to agent's message queue
                // own_wake_data is now NULL
                
                // Process the wake message
                ar__system__process_next_message();
            }
        }
        return initial_agent;
    }
    
    return 0;
}

void ar__system__shutdown(void) {
    if (!is_initialized) {
        return;
    }
    
    // Save methods to file
    ar_methodology_save_methods();
    
    // Save persistent agents to file
    ar_agency_save_agents();
    
    // The memory cleanup is now responsibility of the agency module
    
    // Reset the agency to clean up all agents before disabling
    ar_agency_reset();
    
    // Clean up methodology resources
    ar_methodology_cleanup();
    
    // Now mark as uninitialized
    is_initialized = false;
    ar_agency_set_initialized(false);
}

bool ar__system__process_next_message(void) {
    if (!is_initialized) {
        printf("DEBUG: System not initialized\n");
        return false;
    }
    
    // Find an agent with a non-empty message queue
    int64_t agent_id = ar_agency_get_first_agent();
    printf("DEBUG: First agent ID: %lld\n", (long long)agent_id);
    while (agent_id != 0) {
        if (ar_agency_agent_has_messages(agent_id)) {
            printf("DEBUG: Agent %lld has messages\n", (long long)agent_id);
            // Process one message
            data_t *own_message = ar_agency_get_agent_message(agent_id);
            if (own_message) {
                printf("DEBUG: Got message from agent %lld\n", (long long)agent_id);
                // Get the agent's method
                const method_t *ref_method = ar_agency_get_agent_method(agent_id);
                if (ref_method) {
                    printf("DEBUG: Agent has method\n");
                    // Print message based on its type
                    printf("Agent %lld received message: ", (long long)agent_id);
                    data_type_t msg_type = ar_data_get_type(own_message);
                    if (msg_type == DATA_STRING) {
                        printf("%s\n", ar_data_get_string(own_message));
                    } else if (msg_type == DATA_INTEGER) {
                        printf("%d\n", ar_data_get_integer(own_message));
                    } else if (msg_type == DATA_DOUBLE) {
                        printf("%f\n", ar_data_get_double(own_message));
                    } else if (msg_type == DATA_LIST || msg_type == DATA_MAP) {
                        printf("[complex data]\n");
                    }
                    
                    ar_method_run(agent_id, (const data_t *)own_message, ar_method_get_instructions(ref_method));
                    
                    // Free the message as it's now been processed
                    ar_data_destroy(own_message);
                    own_message = NULL; // Mark as freed
                    return true;
                }
                
                // Free the message if we couldn't process it
                ar_data_destroy(own_message);
                own_message = NULL; // Mark as freed
            }
        }
        agent_id = ar_agency_get_next_agent(agent_id);
    }
    
    return false; // No messages to process
}

int ar__system__process_all_messages(void) {
    int count = 0;
    
    while (ar__system__process_next_message()) {
        count++;
    }
    
    return count;
}

