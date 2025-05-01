/* Agerun Agent Implementation */
#include "agerun_agent.h"
#include "agerun_agency.h"
#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
#include "agerun_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Constants */

/* Static variables for commonly used messages */
static const char g_sleep_message[] = "__sleep__";

/* Implementation */
agent_id_t ar_agent_create(const char *ref_method_name, version_t version, const data_t *ref_context) {
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
    
    if (agent_idx < 0) {
        printf("Error: Maximum number of agents reached\n");
        // No need to free ref_context as we don't own it
        return 0;
    }
    
    // Find method definition from the method module
    const method_t *ref_method = ar_methodology_get_method(ref_method_name, version);
    
    if (!ref_method) {
        printf("Error: Method %s%s%d not found\n", 
               ref_method_name, version ? " version " : "", version);
        // No need to free ref_context as we don't own it
        return 0;
    }
    
    // Initialize agent structure
    agent_id_t next_agent_id = ar_agency_get_next_id();
    mut_agents[agent_idx].id = next_agent_id;
    ar_agency_set_next_id(next_agent_id + 1);
    
    strncpy(mut_agents[agent_idx].method_name, ref_method_name, MAX_METHOD_NAME_LENGTH - 1);
    mut_agents[agent_idx].method_name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
    mut_agents[agent_idx].method_version = ar_method_get_version(ref_method);
    mut_agents[agent_idx].is_active = true;
    mut_agents[agent_idx].is_persistent = ar_method_is_persistent(ref_method);
    mut_agents[agent_idx].ref_context = ref_context; // Context can be NULL
    // No ownership transfer for context as it's just a borrowed reference
    
    // Create memory as an empty map
    mut_agents[agent_idx].own_memory = ar_data_create_map();
    if (!mut_agents[agent_idx].own_memory) {
        printf("Error: Failed to create memory for agent %lld\n", next_agent_id);
        // No need to free ref_context as we don't own it
        mut_agents[agent_idx].ref_context = NULL; // Remove the reference
        return 0;
    }
    // Ownership of own_memory transferred to agent
    
    // Create message queue using list
    mut_agents[agent_idx].own_message_queue = ar_list_create();
    if (!mut_agents[agent_idx].own_message_queue) {
        printf("Error: Failed to create message queue for agent %lld\n", next_agent_id);
        ar_data_destroy(mut_agents[agent_idx].own_memory);
        mut_agents[agent_idx].own_memory = NULL; // Mark as no longer owned
        
        // No need to free ref_context as we don't own it
        mut_agents[agent_idx].ref_context = NULL; // Remove the reference
        
        mut_agents[agent_idx].is_active = false;
        return 0;
    }
    // Ownership of own_message_queue transferred to agent
    
    printf("Created agent %lld using method %s version %d\n", 
           mut_agents[agent_idx].id, ref_method_name, ar_method_get_version(ref_method));
    
    return mut_agents[agent_idx].id; // Ownership transferred to caller
}

bool ar_agent_destroy(agent_id_t agent_id) {
    agent_t *mut_agents = ar_agency_get_agents();
    
    if (mut_agents == NULL || agent_id == 0) {
        return false;
    }
    
    // Find the agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (mut_agents[i].is_active && mut_agents[i].id == agent_id) {
            // Send sleep message before destroying
            data_t *own_sleep_data = ar_data_create_string(g_sleep_message);
            if (own_sleep_data) {
                ar_agent_send(agent_id, own_sleep_data);
                own_sleep_data = NULL; // Mark as transferred
            }
            
            // Process the sleep message
            data_t *own_message = ar_list_remove_first(mut_agents[i].own_message_queue);
            if (own_message) {
                // Find the method definition to process the sleep message
                const method_t *ref_method = ar_methodology_get_method(mut_agents[i].method_name, mut_agents[i].method_version);
                if (ref_method) {
                    // Print message based on its type
                    printf("Agent %lld received message: ", mut_agents[i].id);
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
                    
                    ar_method_run(&mut_agents[i], (const data_t *)own_message, ar_method_get_instructions(ref_method));
                }
                
                // Free the message data
                ar_data_destroy(own_message);
                own_message = NULL; // Mark as destroyed
            }
            
            // Free memory data if it exists
            if (mut_agents[i].own_memory) {
                ar_data_destroy(mut_agents[i].own_memory);
                mut_agents[i].own_memory = NULL; // Mark as destroyed
            }
            
            // Clear context reference (we don't own it)
            mut_agents[i].ref_context = NULL;
            
            // Free message queue if it exists
            if (mut_agents[i].own_message_queue) {
                ar_list_destroy(mut_agents[i].own_message_queue);
                mut_agents[i].own_message_queue = NULL; // Mark as destroyed
            }
            
            mut_agents[i].is_active = false;
            printf("Destroyed agent %lld\n", agent_id);
            return true;
        }
    }
    
    return false;
}

bool ar_agent_send(agent_id_t agent_id, data_t *own_message) {
    agent_t *mut_agents = ar_agency_get_agents();
    
    if (mut_agents == NULL || !own_message) {
        // Free own_message if provided since we're failing
        if (own_message) {
            ar_data_destroy(own_message);
        }
        return false;
    }
    
    // Special case: agent_id 0 is a no-op
    if (agent_id == 0) {
        // Free the message since we're not using it
        ar_data_destroy(own_message);
        // Mark as destroyed since we took ownership
        own_message = NULL;
        return true;
    }
    
    // Find the agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (mut_agents[i].is_active && mut_agents[i].id == agent_id) {
            // Add message to queue - list borrows the reference but doesn't take ownership
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

bool ar_agent_exists(agent_id_t agent_id) {
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

/* End of implementation */
