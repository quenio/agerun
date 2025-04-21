/* Agerun Agency Implementation */
#include "agerun_agency.h"
#include "agerun_agent.h"
#include "agerun_map.h"
#include "agerun_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constants */

/* Global State */
static agent_t agents[MAX_AGENTS];
static agent_id_t next_agent_id = 1;
static bool is_initialized = false;

/* Static initialization */
static void ar_agency_init(void) {
    if (!is_initialized) {
        for (int i = 0; i < MAX_AGENTS; i++) {
            agents[i].is_active = false;
            agents[i].memory = NULL;
            agents[i].queue = NULL;
            agents[i].context = NULL;
        }
        is_initialized = true;
    }
}

/* Implementation */
void ar_agency_set_initialized(bool initialized) {
    is_initialized = initialized;
}

agent_t* ar_agency_get_agents(void) {
    if (!is_initialized) {
        ar_agency_init();
    }
    return agents;
}

agent_id_t ar_agency_get_next_id(void) {
    return next_agent_id;
}

void ar_agency_set_next_id(agent_id_t id) {
    next_agent_id = id;
}

void ar_agency_reset(void) {
    // Reset all agents to inactive
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (agents[i].is_active) {
            if (agents[i].memory) {
                ar_data_destroy(agents[i].memory);
            }
            // Only destroy context if it exists
            if (agents[i].context) {
                ar_data_destroy(agents[i].context);
            }
            if (agents[i].queue) {
                ar_queue_destroy(agents[i].queue);
            }
        }
        agents[i].is_active = false;
        agents[i].memory = NULL;
        agents[i].context = NULL;
        agents[i].queue = NULL;
    }
    
    // Reset next_agent_id
    next_agent_id = 1;
}

int ar_agency_count_agents(void) {
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

bool ar_agency_save_agents(void) {
    if (!is_initialized) {
        return false;
    }
    
    // Simple placeholder implementation for now
    FILE *fp = fopen(AGENCY_FILE_NAME, "w");
    if (!fp) {
        printf("Error: Could not open %s for writing\n", AGENCY_FILE_NAME);
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
            
            // Save memory map placeholder
            // For now, just save an empty count since we can't access the internal structure
            fprintf(fp, "0\n");
            // In a complete implementation, we would iterate over the map entries
            // using a new function like ar_map_for_each() to process each key/value pair
        }
    }
    
    fclose(fp);
    return true;
}

bool ar_agency_load_agents(void) {
    if (!is_initialized) {
        return false;
    }
    
    FILE *fp = fopen(AGENCY_FILE_NAME, "r");
    if (!fp) {
        // Not an error, might be first run
        return true;
    }
    
    int count = 0;
    if (fscanf(fp, "%d", &count) != 1) {
        fclose(fp);
        return false;
    }
    
    for (int i = 0; i < count; i++) {
        agent_id_t id;
        char method_name[MAX_METHOD_NAME_LENGTH];
        version_t version;
        
        if (fscanf(fp, "%lld %s %d", &id, method_name, &version) != 3) {
            printf("Error: Malformed agent entry in %s\n", AGENCY_FILE_NAME);
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
        for (int j = 0; j < MAX_AGENTS; j++) {
            if (agents[j].is_active && agents[j].id == new_id) {
                agents[j].id = id;
                
                // Read memory map
                int mem_count = 0;
                if (fscanf(fp, "%d", &mem_count) != 1) {
                    printf("Error: Could not read memory count\n");
                    break;
                }
                
                for (int k = 0; k < mem_count; k++) {
                    char key[256];
                    char type[32];
                    
                    if (fscanf(fp, "%255s %31s", key, type) != 2) {
                        printf("Error: Malformed memory entry in %s\n", AGENCY_FILE_NAME);
                        break;
                    }
                    
                    data_t *value = NULL;
                    if (strcmp(type, "int") == 0) {
                        int int_value;
                        if (fscanf(fp, "%d", &int_value) != 1) {
                            printf("Error: Could not read int value\n");
                            break;
                        }
                        value = ar_data_create_integer(int_value);
                    } else if (strcmp(type, "double") == 0) {
                        double double_value;
                        if (fscanf(fp, "%lf", &double_value) != 1) {
                            printf("Error: Could not read double value\n");
                            break;
                        }
                        value = ar_data_create_double(double_value);
                    } else if (strcmp(type, "string") == 0) {
                        char str[1024];
                        if (fscanf(fp, "%1023s", str) != 1) {
                            printf("Error: Could not read string value\n");
                            break;
                        }
                        value = ar_data_create_string(str);
                    } else {
                        // Skip unknown type
                        char line[1024];
                        fgets(line, sizeof(line), fp);
                        continue;
                    }
                    
                    if (value) {
                        ar_data_set_map_data(agents[j].memory, key, value);
                        // Note: Data ownership is transferred, so we don't free value here
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

/* End of implementation */

