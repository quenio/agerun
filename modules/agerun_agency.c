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
static agent_t g_own_agents[MAX_AGENTS]; // Owned by the agency module
static agent_id_t g_next_agent_id = 1;
static bool g_is_initialized = false;

/* Static initialization */
static void ar_agency_init(void) {
    if (!g_is_initialized) {
        for (int i = 0; i < MAX_AGENTS; i++) {
            g_own_agents[i].is_active = false;
            g_own_agents[i].own_memory = NULL;
            g_own_agents[i].own_message_queue = NULL;
            g_own_agents[i].ref_context = NULL; // Not owned, just initialize reference
        }
        g_is_initialized = true;
    }
}

/* Implementation */
void ar_agency_set_initialized(bool initialized) {
    g_is_initialized = initialized;
}

agent_t* ar_agency_get_agents(void) {
    if (!g_is_initialized) {
        ar_agency_init();
    }
    return g_own_agents; // Ownership: Borrowed reference (module retains ownership)
}

agent_id_t ar_agency_get_next_id(void) {
    return g_next_agent_id; // Value type, not a reference
}

void ar_agency_set_next_id(agent_id_t id) {
    g_next_agent_id = id; // Value type, not a reference
}

void ar_agency_reset(void) {
    // Reset all agents to inactive
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active) {
            // Free memory data if it exists
            if (g_own_agents[i].own_memory) {
                ar_data_destroy(g_own_agents[i].own_memory);
                g_own_agents[i].own_memory = NULL; // Mark as destroyed
            }
            
            // Clear context reference (we don't own it)
            g_own_agents[i].ref_context = NULL;
            
            // Free message queue if it exists
            if (g_own_agents[i].own_message_queue) {
                ar_list_destroy(g_own_agents[i].own_message_queue);
                g_own_agents[i].own_message_queue = NULL; // Mark as destroyed
            }
        }
        g_own_agents[i].is_active = false;
    }
    
    // Reset next_agent_id
    g_next_agent_id = 1;
}

int ar_agency_count_agents(void) {
    if (!g_is_initialized) {
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active) {
            count++;
        }
    }
    
    return count; // Value type, not a reference
}

bool ar_agency_save_agents(void) {
    if (!g_is_initialized) {
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
        if (g_own_agents[i].is_active && g_own_agents[i].is_persistent) {
            count++;
        }
    }
    
    fprintf(fp, "%d\n", count);
    
    // Save basic agent info
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].is_persistent) {
            fprintf(fp, "%lld %s %d\n", g_own_agents[i].id, g_own_agents[i].method_name, g_own_agents[i].method_version);
            
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
    if (!g_is_initialized) {
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
        // Note: The ar_agent_create function takes ownership of any context passed
        
        // Update the assigned ID to match the stored one
        for (int j = 0; j < MAX_AGENTS; j++) {
            if (g_own_agents[j].is_active && g_own_agents[j].id == new_id) {
                g_own_agents[j].id = id;
                
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
                    
                    data_t *own_value = NULL; // Will be an owned value after creation
                    if (strcmp(type, "int") == 0) {
                        int int_value;
                        if (fscanf(fp, "%d", &int_value) != 1) {
                            printf("Error: Could not read int value\n");
                            break;
                        }
                        own_value = ar_data_create_integer(int_value);
                    } else if (strcmp(type, "double") == 0) {
                        double double_value;
                        if (fscanf(fp, "%lf", &double_value) != 1) {
                            printf("Error: Could not read double value\n");
                            break;
                        }
                        own_value = ar_data_create_double(double_value);
                    } else if (strcmp(type, "string") == 0) {
                        char str[1024];
                        if (fscanf(fp, "%1023s", str) != 1) {
                            printf("Error: Could not read string value\n");
                            break;
                        }
                        own_value = ar_data_create_string(str);
                    } else {
                        // Skip unknown type
                        char line[1024];
                        fgets(line, sizeof(line), fp);
                        continue;
                    }
                    
                    if (own_value) {
                        ar_data_set_map_data(g_own_agents[j].own_memory, key, own_value);
                        // Note: Data ownership is transferred, so we don't free value here
                        own_value = NULL; // Mark as transferred
                    }
                }
                
                break;
            }
        }
        
        // Update next_agent_id if needed
        if (id >= g_next_agent_id) {
            g_next_agent_id = id + 1;
        }
    }
    
    fclose(fp);
    return true;
}

/* End of implementation */

