/* Agerun Runtime System Implementation */
#include "agerun_system.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
#include "agerun_data.h"
#include "agerun_agent.h"
#include "agerun_agency.h"
#include "agerun_queue.h"
#include "agerun_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/* Constants */
#define MAX_METHODS 256
#define MAX_VERSIONS_PER_METHOD 64
#define MAX_METHOD_NAME_LENGTH 64
#define MAX_MESSAGE_LENGTH 1024
#define MAX_INSTRUCTIONS_LENGTH 16384
#define QUEUE_SIZE 256

/* Memory Map structure is now defined in agerun_map.h */

/* Message Queue is now defined in agerun_queue.h */

/* Method Definition is now in agerun_method.h/c */

/* Agent Definition is now in agerun_agent.h */

/* Global State */
static bool is_initialized = false;

/* Implementation */
agent_id_t ar_init(const char *method_name, version_t version) {
    if (is_initialized) {
        printf("Agerun already initialized\n");
        return 0;
    }
    
    is_initialized = true;
    ar_agency_set_initialized(true);
    
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
        agent_id_t initial_agent = ar_agent_create(method_name, version, NULL);
        if (initial_agent != 0) {
            // Send wake message to initial agent
            ar_agent_send(initial_agent, "__wake__");
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
    agent_t* agents = ar_agency_get_agents();
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
    ar_agency_reset();
    ar_agency_set_initialized(false);
}



bool ar_process_next_message(void) {
    if (!is_initialized) {
        return false;
    }
    
    // Find an agent with a non-empty message queue
    agent_t* agents = ar_agency_get_agents();
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active && agents[i].queue.size > 0) {
            // Process one message
            char message[MAX_MESSAGE_LENGTH];
            if (ar_queue_pop(&agents[i].queue, message)) {
                // Use the interpret_method function from agerun_agent
                // Since that's now private, we need to call the method directly
                method_t *method = ar_method_get(agents[i].method_name, agents[i].method_version);
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

int ar_process_all_messages(void) {
    int count = 0;
    
    while (ar_process_next_message()) {
        count++;
    }
    
    return count;
}

/* This function is now implemented in agerun_agent.c */

int ar_count_agents(void) {
    if (!is_initialized) {
        return 0;
    }
    
    int count = 0;
    agent_t* agents = ar_agency_get_agents();
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
    
    agent_t* agents = ar_agency_get_agents();
    
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
        agent_id_t new_id = ar_agent_create(method_name, version, NULL);
        if (new_id == 0) {
            printf("Error: Could not recreate agent %lld\n", id);
            continue;
        }
        
        // Update the assigned ID to match the stored one
        agent_t* agents = ar_agency_get_agents();
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
        agent_id_t next_id = ar_agency_get_next_id();
        if (id >= next_id) {
            ar_agency_set_next_id(id + 1);
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



/* Memory functions are now defined in agerun_map.c */

// Memory functions are now defined in agerun_map.c
