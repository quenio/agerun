/* Agerun Agent Implementation */
#include "agerun_agent.h"
#include "agerun_agency.h"
#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_map.h"
#include "agerun_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Constants */
#define MAX_MESSAGE_LENGTH 1024

/* Implementation */
agent_id_t ar_agent_create(const char *method_name, version_t version, void *context) {
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
    method_t *method = ar_method_get(method_name, version);
    
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
    agents[agent_idx].context = (map_t *)context;
    
    ar_map_init(&agents[agent_idx].memory);
    ar_queue_init(&agents[agent_idx].queue);
    
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
            ar_agent_send(agent_id, "__sleep__");
            
            // Process the sleep message
            char message[MAX_MESSAGE_LENGTH];
            if (ar_queue_pop(&agents[i].queue, message)) {
                // Find the method definition to process the sleep message
                method_t *method = ar_method_get(agents[i].method_name, agents[i].method_version);
                if (method) {
                    printf("Agent %lld received message: %s\n", agents[i].id, message);
                    ar_method_run(&agents[i], message, method->instructions);
                }
            }
            
            // Free memory map entries
            for (int j = 0; j < MAP_SIZE; j++) {
                if (agents[i].memory.entries[j].is_used && agents[i].memory.entries[j].key) {
                    free(agents[i].memory.entries[j].key);
                    ar_data_free(&agents[i].memory.entries[j].value);
                }
            }
            
            agents[i].is_active = false;
            printf("Destroyed agent %lld\n", agent_id);
            return true;
        }
    }
    
    return false;
}

bool ar_agent_send(agent_id_t agent_id, const char *message) {
    agent_t *agents = ar_agency_get_agents();
    
    if (agents == NULL || !message) {
        return false;
    }
    
    // Special case: agent_id 0 is a no-op
    if (agent_id == 0) {
        return true;
    }
    
    // Find the agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].id == agent_id) {
            // Add message to queue
            return ar_queue_push(&agents[i].queue, message);
        }
    }
    
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
