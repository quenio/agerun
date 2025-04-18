/* Agerun Runtime System Implementation */
#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_data.h"
#include "agerun_agent.h"
#include "agerun_queue.h"
#include "agerun_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/* Constants */
#define MAX_AGENTS 1024
#define MAX_METHODS 256
#define MAX_VERSIONS_PER_METHOD 64
#define MAX_METHOD_NAME_LENGTH 64
#define MAX_MESSAGE_LENGTH 1024
#define MAX_INSTRUCTIONS_LENGTH 16384
// MAP_SIZE is now defined in agerun_map.h
#define QUEUE_SIZE 256

/* Memory Map structure is now defined in agerun_map.h */

/* Message Queue is now defined in agerun_queue.h */

/* Method Definition is now in agerun_method.h/c */

/* Agent Definition is now in agerun_agent.h */

/* Global State */
static agent_t agents[MAX_AGENTS];
static agent_id_t next_agent_id = 1;
static bool is_initialized = false;

/* Forward Declarations */
static bool interpret_method(agent_t *agent, const char *message);

/* Implementation */
agent_id_t ar_init(const char *method_name, version_t version) {
    if (is_initialized) {
        printf("Agerun already initialized\n");
        return 0;
    }
    
    // Initialize all agents as inactive
    for (int i = 0; i < MAX_AGENTS; i++) {
        agents[i].is_active = false;
    }
    
    is_initialized = true;
    
    // Load methods from file if available
    if (!ar_load_methods()) {
        printf("Warning: Could not load methods from file\n");
    }
    
    // Load agents from file if available
    if (!ar_load_agents()) {
        printf("Warning: Could not load agents from file\n");
    }
    
    // Create initial agent if method_name is provided
    if (method_name != NULL) {
        agent_id_t initial_agent = ar_create(method_name, version, NULL);
        if (initial_agent != 0) {
            // Send wake message to initial agent
            ar_send(initial_agent, "__wake__");
        }
        return initial_agent;
    }
    
    return 0;
}

void ar_shutdown(void) {
    if (!is_initialized) {
        return;
    }
    
    // Save methods to file
    ar_save_methods();
    
    // Save persistent agents to file
    ar_save_agents();
    
    // Clean up memory for all active agents
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active) {
            // Free memory map entries
            for (int j = 0; j < MAP_SIZE; j++) {
                if (agents[i].memory.entries[j].is_used && agents[i].memory.entries[j].key) {
                    free(agents[i].memory.entries[j].key);
                    ar_data_free(&agents[i].memory.entries[j].value);
                }
            }
        }
    }
    
    is_initialized = false;
}


agent_id_t ar_create(const char *method_name, version_t version, void *context) {
    if (!is_initialized || !method_name) {
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
    agents[agent_idx].id = next_agent_id++;
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

bool ar_destroy(agent_id_t agent_id) {
    if (!is_initialized || agent_id == 0) {
        return false;
    }
    
    // Find the agent
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].id == agent_id) {
            // Send sleep message before destroying
            ar_send(agent_id, "__sleep__");
            
            // Process the sleep message
            char message[MAX_MESSAGE_LENGTH];
            if (ar_queue_pop(&agents[i].queue, message)) {
                interpret_method(&agents[i], message);
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

bool ar_send(agent_id_t agent_id, const char *message) {
    if (!is_initialized || !message) {
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

bool ar_process_next_message(void) {
    // Find an agent with a non-empty message queue
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].queue.size > 0) {
            // Process one message
            char message[MAX_MESSAGE_LENGTH];
            if (ar_queue_pop(&agents[i].queue, message)) {
                interpret_method(&agents[i], message);
                return true;
            }
        }
    }
    
    return false; // No messages to process
}

int ar_process_all_messages(void) {
    int count = 0;
    
    while (ar_process_next_message()) {
        count++;
    }
    
    return count;
}

bool ar_agent_exists(agent_id_t agent_id) {
    if (!is_initialized) {
        return false;
    }
    
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].id == agent_id) {
            return true;
        }
    }
    
    return false;
}

int ar_count_agents(void) {
    if (!is_initialized) {
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active) {
            count++;
        }
    }
    
    return count;
}

bool ar_save_agents(void) {
    if (!is_initialized) {
        return false;
    }
    
    // Simple placeholder implementation for now
    FILE *fp = fopen("agrun.agents", "w");
    if (!fp) {
        printf("Error: Could not open agrun.agents for writing\n");
        return false;
    }
    
    // Count how many persistent agents we have
    int count = 0;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].is_persistent) {
            count++;
        }
    }
    
    fprintf(fp, "%d\n", count);
    
    // Save basic agent info
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].is_persistent) {
            fprintf(fp, "%lld %s %d\n", agents[i].id, agents[i].method_name, agents[i].method_version);
            
            // Save memory map - for simplicity just save int and string values
            fprintf(fp, "%d\n", agents[i].memory.count);
            for (int j = 0; j < MAP_SIZE; j++) {
                if (agents[i].memory.entries[j].is_used && agents[i].memory.entries[j].key) {
                    fprintf(fp, "%s ", agents[i].memory.entries[j].key);
                    
                    data_t *val = &agents[i].memory.entries[j].value;
                    if (val->type == DATA_INT) {
                        fprintf(fp, "int %lld\n", val->data.int_value);
                    } else if (val->type == DATA_DOUBLE) {
                        fprintf(fp, "double %f\n", val->data.double_value);
                    } else if (val->type == DATA_STRING && val->data.string_value) {
                        fprintf(fp, "string %s\n", val->data.string_value);
                    } else {
                        fprintf(fp, "unknown\n");
                    }
                }
            }
        }
    }
    
    fclose(fp);
    return true;
}

bool ar_load_agents(void) {
    if (!is_initialized) {
        return false;
    }
    
    FILE *fp = fopen("agrun.agents", "r");
    if (!fp) {
        // Not an error, might be first run
        return true;
    }
    
    int count = 0;
    fscanf(fp, "%d", &count);
    
    for (int i = 0; i < count; i++) {
        agent_id_t id;
        char method_name[MAX_METHOD_NAME_LENGTH];
        version_t version;
        
        if (fscanf(fp, "%lld %s %d", &id, method_name, &version) != 3) {
            printf("Error: Malformed agent entry in agrun.agents\n");
            fclose(fp);
            return false;
        }
        
        // Create the agent
        agent_id_t new_id = ar_create(method_name, version, NULL);
        if (new_id == 0) {
            printf("Error: Could not recreate agent %lld\n", id);
            continue;
        }
        
        // Update the assigned ID to match the stored one
        for (int j = 0; j < MAX_AGENTS; j++) {
            if (agents[j].is_active && agents[j].id == new_id) {
                agents[j].id = id;
                
                // Read memory map
                int mem_count = 0;
                fscanf(fp, "%d", &mem_count);
                
                for (int k = 0; k < mem_count; k++) {
                    char key[256];
                    char type[32];
                    
                    if (fscanf(fp, "%255s %31s", key, type) != 2) {
                        printf("Error: Malformed memory entry in agrun.agents\n");
                        break;
                    }
                    
                    data_t value;
                    if (strcmp(type, "int") == 0) {
                        value.type = DATA_INT;
                        fscanf(fp, "%lld", &value.data.int_value);
                    } else if (strcmp(type, "double") == 0) {
                        value.type = DATA_DOUBLE;
                        fscanf(fp, "%lf", &value.data.double_value);
                    } else if (strcmp(type, "string") == 0) {
                        value.type = DATA_STRING;
                        char str[1024];
                        fscanf(fp, "%1023s", str);
                        value.data.string_value = strdup(str);
                    } else {
                        // Skip unknown type
                        char line[1024];
                        fgets(line, sizeof(line), fp);
                        continue;
                    }
                    
                    ar_map_set(&agents[j].memory, key, &value);
                    
                    if (value.type == DATA_STRING && value.data.string_value) {
                        free(value.data.string_value);
                    }
                }
                
                break;
            }
        }
        
        // Update next_agent_id if needed
        if (id >= next_agent_id) {
            next_agent_id = id + 1;
        }
    }
    
    fclose(fp);
    return true;
}

bool ar_save_methods(void) {
    // TODO: This function needs to be reimplemented to use the method module APIs
    if (!is_initialized) {
        return false;
    }
    
    printf("Method saving not implemented yet\n");
    return false;
}

bool ar_load_methods(void) {
    // TODO: This function needs to be reimplemented to use the method module APIs
    if (!is_initialized) {
        return false;
    }
    
    printf("Method loading not implemented yet\n");
    return false;
}

// Memory functions are now defined in agerun_map.c

// This function has been moved to agerun_value.c

/* Queue functions are now defined in agerun_message.c */



static bool interpret_method(agent_t *agent, const char *message) {
    // Find the method - using the method module's exported function
    method_t *method = ar_method_get(agent->method_name, agent->method_version);
    if (!method) {
        printf("Error: Method %s version %d not found for agent %lld\n", 
               agent->method_name, agent->method_version, agent->id);
        return false;
    }
    
    printf("Agent %lld received message: %s\n", agent->id, message);
    
    // Call the interpreter function with the method instructions
    return ar_method_run(agent, message, method->instructions);
}

// Memory functions are now defined in agerun_map.c
