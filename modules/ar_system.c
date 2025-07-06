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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>

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
static interpreter_t *g_interpreter = NULL;

/* Implementation */
int64_t ar_system__init(const char *ref_method_name, const char *ref_version) {
    if (is_initialized) {
        printf("Agerun already initialized\n");
        return 0;
    }
    
    is_initialized = true;
    ar_agency__set_initialized(true);
    
    // Create the interpreter
    g_interpreter = ar_interpreter__create();
    if (!g_interpreter) {
        printf("Error: Failed to create interpreter\n");
        is_initialized = false;
        ar_agency__set_initialized(false);
        return 0;
    }
    
    // Load methods from file if available
    if (!ar_methodology__load_methods()) {
        printf("Warning: Could not load methods from file\n");
    }
    
    // Load agents from file if available
    if (!ar_agency__load_agents()) {
        printf("Warning: Could not load agents from file\n");
    }
    
    // Create initial agent if ref_method_name is provided
    if (ref_method_name != NULL) {
        // Create initial agent with NULL context
        int64_t initial_agent = ar_agency__create_agent(ref_method_name, ref_version, NULL);
        if (initial_agent != 0) {
            // Send wake message to initial agent
            ar_data_t *own_wake_data = ar_data__create_string(g_wake_message);
            if (own_wake_data) {
                ar_agency__send_to_agent(initial_agent, own_wake_data);
                // Ownership transferred to agent's message queue
                // own_wake_data is now NULL
                
                // Process the wake message
                ar_system__process_next_message();
            }
        }
        return initial_agent;
    }
    
    return 0;
}

void ar_system__shutdown(void) {
    if (!is_initialized) {
        return;
    }
    
    // Save methods to file
    ar_methodology__save_methods();
    
    // Save persistent agents to file
    ar_agency__save_agents();
    
    // The memory cleanup is now responsibility of the agency module
    
    // Reset the agency to clean up all agents before disabling
    ar_agency__reset();
    
    // Clean up methodology resources
    ar_methodology__cleanup();
    
    // Destroy the interpreter
    if (g_interpreter) {
        ar_interpreter__destroy(g_interpreter);
        g_interpreter = NULL;
    }
    
    // Now mark as uninitialized
    is_initialized = false;
    ar_agency__set_initialized(false);
}

bool ar_system__process_next_message(void) {
    if (!is_initialized) {
        printf("DEBUG: System not initialized\n");
        return false;
    }
    
    // Find an agent with a non-empty message queue
    int64_t agent_id = ar_agency__get_first_agent();
    printf("DEBUG: First agent ID: %" PRId64 "\n", agent_id);
    while (agent_id != 0) {
        if (ar_agency__agent_has_messages(agent_id)) {
            printf("DEBUG: Agent %" PRId64 " has messages\n", agent_id);
            // Process one message
            ar_data_t *own_message = ar_agency__get_agent_message(agent_id);
            if (own_message) {
                printf("DEBUG: Got message from agent %" PRId64 "\n", agent_id);
                // Get the agent's method
                const method_t *ref_method = ar_agency__get_agent_method(agent_id);
                if (ref_method) {
                    printf("DEBUG: Agent has method\n");
                    // Print message based on its type
                    printf("Agent %" PRId64 " received message: ", agent_id);
                    ar_data_type_t msg_type = ar_data__get_type(own_message);
                    if (msg_type == DATA_STRING) {
                        printf("%s\n", ar_data__get_string(own_message));
                    } else if (msg_type == DATA_INTEGER) {
                        printf("%d\n", ar_data__get_integer(own_message));
                    } else if (msg_type == DATA_DOUBLE) {
                        printf("%f\n", ar_data__get_double(own_message));
                    } else if (msg_type == DATA_LIST || msg_type == DATA_MAP) {
                        printf("[complex data]\n");
                    }
                    
                    ar_interpreter__execute_method(g_interpreter, agent_id, own_message, ref_method);
                    
                    // Free the message as it's now been processed
                    ar_data__destroy(own_message);
                    own_message = NULL; // Mark as freed
                    return true;
                }
                
                // Free the message if we couldn't process it
                ar_data__destroy(own_message);
                own_message = NULL; // Mark as freed
            }
        }
        agent_id = ar_agency__get_next_agent(agent_id);
    }
    
    return false; // No messages to process
}

int ar_system__process_all_messages(void) {
    int count = 0;
    
    while (ar_system__process_next_message()) {
        count++;
    }
    
    return count;
}

