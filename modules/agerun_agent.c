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
static char g_sleep_message[] = "__sleep__";

/* Implementation */
agent_id_t ar_agent_create(const char *method_name, version_t version, data_t *context) {
    agent_t *agents = ar_agency_get_agents();
    
    if (agents == NULL || !method_name) {
        return 0;
    }
    
    // Find free slot for new agent
    int agent_idx = -1;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (!agents[i].is_active) {
            agent_idx = i;
            break;
        }
    }
    
    if (agent_idx < 0) {
        printf("Error: Maximum number of agents reached\n");
        return 0;
    }
    
    // Find method definition from the method module
    method_t *method = ar_methodology_get_method(method_name, version);
    
    if (!method) {
        printf("Error: Method %s%s%d not found\n", 
               method_name, version ? " version " : "", version);
        return 0;
    }
    
    // Initialize agent structure
    agent_id_t next_agent_id = ar_agency_get_next_id();
    agents[agent_idx].id = next_agent_id;
    ar_agency_set_next_id(next_agent_id + 1);
    
    strncpy(agents[agent_idx].method_name, method_name, MAX_METHOD_NAME_LENGTH - 1);
    agents[agent_idx].method_name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
    agents[agent_idx].method_version = method->version;
    agents[agent_idx].is_active = true;
    agents[agent_idx].is_persistent = method->persist;
    agents[agent_idx].context = context; // Context can be NULL
    
    // Create memory as an empty map
    agents[agent_idx].memory = ar_data_create_map();
    if (!agents[agent_idx].memory) {
        printf("Error: Failed to create memory for agent %lld\n", next_agent_id);
        return 0;
    }
    
    // Create message queue using list
    agents[agent_idx].message_queue = ar_list_create();
    if (!agents[agent_idx].message_queue) {
        printf("Error: Failed to create message queue for agent %lld\n", next_agent_id);
        ar_data_destroy(agents[agent_idx].memory);
        agents[agent_idx].is_active = false;
        return 0;
    }
    
    printf("Created agent %lld using method %s version %d\n", 
           agents[agent_idx].id, method_name, method->version);
    
    return agents[agent_idx].id;
}

bool ar_agent_destroy(agent_id_t agent_id) {
    agent_t *agents = ar_agency_get_agents();
    
    if (agents == NULL || agent_id == 0) {
        return false;
    }
    
    // Find the agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].id == agent_id) {
            // Send sleep message before destroying
            data_t *sleep_data = ar_data_create_string(g_sleep_message);
            if (sleep_data) {
                ar_agent_send(agent_id, sleep_data);
            }
            
            // Process the sleep message
            data_t *message = ar_list_remove_first(agents[i].message_queue);
            if (message) {
                // Find the method definition to process the sleep message
                method_t *method = ar_methodology_get_method(agents[i].method_name, agents[i].method_version);
                if (method) {
                    // Print message based on its type
                    printf("Agent %lld received message: ", agents[i].id);
                    data_type_t msg_type = ar_data_get_type(message);
                    if (msg_type == DATA_STRING) {
                        printf("%s\n", ar_data_get_string(message));
                    } else if (msg_type == DATA_INTEGER) {
                        printf("%d\n", ar_data_get_integer(message));
                    } else if (msg_type == DATA_DOUBLE) {
                        printf("%f\n", ar_data_get_double(message));
                    } else if (msg_type == DATA_LIST || msg_type == DATA_MAP) {
                        printf("[complex data]\n");
                    }
                    
                    ar_method_run(&agents[i], message, method->instructions);
                }
                
                // Free the message data
                ar_data_destroy(message);
            }
            
            // Free memory data if it exists
            if (agents[i].memory) {
                ar_data_destroy(agents[i].memory);
                agents[i].memory = NULL;
            }
            
            // Free context data if it exists
            if (agents[i].context) {
                ar_data_destroy(agents[i].context);
                agents[i].context = NULL;
            }
            
            // Free message queue if it exists
            if (agents[i].message_queue) {
                ar_list_destroy(agents[i].message_queue);
                agents[i].message_queue = NULL;
            }
            
            agents[i].is_active = false;
            printf("Destroyed agent %lld\n", agent_id);
            return true;
        }
    }
    
    return false;
}

bool ar_agent_send(agent_id_t agent_id, data_t *message) {
    agent_t *agents = ar_agency_get_agents();
    
    if (agents == NULL || !message) {
        return false;
    }
    
    // Special case: agent_id 0 is a no-op
    if (agent_id == 0) {
        // Free the message since we're not using it
        ar_data_destroy(message);
        return true;
    }
    
    // Find the agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].id == agent_id) {
            // Add message to queue
            bool result = ar_list_add_last(agents[i].message_queue, message);
            
            // If we couldn't add to the queue, destroy the message
            if (!result) {
                ar_data_destroy(message);
            }
            
            return result;
        }
    }
    
    // If we couldn't find the agent, destroy the message
    ar_data_destroy(message);
    return false;
}

bool ar_agent_exists(agent_id_t agent_id) {
    agent_t *agents = ar_agency_get_agents();
    
    if (agents == NULL) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].id == agent_id) {
            return true;
        }
    }
    
    return false;
}

/* End of implementation */
