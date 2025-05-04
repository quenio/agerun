/* Agerun Agency Implementation */
#include "agerun_agency.h"
#include "agerun_agent.h"
#include "agerun_map.h"
#include "agerun_data.h"
#include "agerun_method.h"
#include "agerun_semver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constants */

/* Global State */
static agent_t g_own_agents[MAX_AGENTS]; // Owned by the agency module
static agent_id_t g_next_agent_id = 1;
static bool g_is_initialized = false;

/* Helper function for safe file reading */
static bool ar_agency_safe_read_line(FILE *fp, char *buffer, int buffer_size, const char *filename) {
    // Initialize buffer with empty string
    if (buffer_size > 0) {
        buffer[0] = '\0';
    } else {
        return false;
    }
    
    // First check for EOF
    if (feof(fp)) {
        printf("Error: Unexpected end of file in %s\n", filename);
        return false;
    }
    
    // Check for errors and clear them if present
    clearerr(fp);
    
    // Read character by character to avoid the file position warning
    int i = 0;
    int c;
    
    while (i < buffer_size - 1 && (c = fgetc(fp)) != EOF && c != '\n') {
        buffer[i++] = (char)c;
    }
    
    // Add null terminator
    buffer[i] = '\0';
    
    // Add newline if we stopped because of it
    if (c == '\n' && i < buffer_size - 1) {
        buffer[i] = '\n';
        buffer[i+1] = '\0';
    }
    
    // Check for errors
    if (ferror(fp)) {
        printf("Error: Failed to read file %s\n", filename);
        clearerr(fp);
        return false;
    }
    
    // Check if we got any characters
    if (i == 0 && c == EOF) {
        printf("Error: Unexpected end of file in %s\n", filename);
        return false;
    }
    
    return true;
}

/* Static initialization */
static void ar_agency_init(void) {
    if (!g_is_initialized) {
        for (int i = 0; i < MAX_AGENTS; i++) {
            g_own_agents[i].is_active = false;
            g_own_agents[i].ref_method = NULL;
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
    
    // Count how many active agents we have - we don't have persistence flag anymore,
    // so just save all active agents
    int count = 0;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].ref_method != NULL) {
            count++;
        }
    }
    
    // Use snprintf to safely format the output
    char buffer[128];
    int written = snprintf(buffer, sizeof(buffer), "%d\n", count);
    if (written < 0 || written >= (int)sizeof(buffer) || fputs(buffer, fp) == EOF) {
        fclose(fp);
        return false;
    }
    
    // Save basic agent info
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].ref_method != NULL) {
            // Get method name and version from the method reference
            const char *method_name = ar_method_get_name(g_own_agents[i].ref_method);
            const char *method_version = ar_method_get_version(g_own_agents[i].ref_method);
            
            // Use snprintf to safely format agent data
            written = snprintf(buffer, sizeof(buffer), "%lld %s %s\n", 
                              g_own_agents[i].id, method_name, method_version);
            if (written < 0 || written >= (int)sizeof(buffer) || fputs(buffer, fp) == EOF) {
                fclose(fp);
                return false;
            }
            
            // Save memory map placeholder
            // For now, just save an empty count since we can't access the internal structure
            if (fputs("0\n", fp) == EOF) {
                fclose(fp);
                return false;
            }
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
    
    // Read the first line to get agent count
    char line[256];
    
    if (!ar_agency_safe_read_line(fp, line, (int)sizeof(line), AGENCY_FILE_NAME)) {
        fclose(fp);
        return false;
    }
    
    int count = 0;
    // Replace sscanf with more secure strtol
    char *line_endptr;
    count = (int)strtol(line, &line_endptr, 10);
    if (line_endptr == line || *line_endptr != '\n' || count < 0) {
        fclose(fp);
        return false;
    }
    
    for (int i = 0; i < count; i++) {
        agent_id_t id;
        char method_name[256] = {0}; // Increased buffer size for safety, initialized to zeros
        char method_version[64] = {0}; // Buffer for semver string, initialized to zeros
        
        if (!ar_agency_safe_read_line(fp, line, (int)sizeof(line), AGENCY_FILE_NAME)) {
            fclose(fp);
            return false;
        }
        
        // Parse line manually instead of using sscanf
        char *token, *next_token = NULL;
        
        // Get the ID
        token = strtok_r(line, " \t\n", &next_token);
        if (token == NULL) {
            printf("Error: Malformed agent entry - missing ID in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        
        // Convert ID from string to agent_id_t (long long)
        char *id_endptr = NULL;
        id = strtoll(token, &id_endptr, 10);
        if (id_endptr == token || *id_endptr != '\0') {
            printf("Error: Malformed agent ID in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        
        // Get the method name
        token = strtok_r(NULL, " \t\n", &next_token);
        if (token == NULL) {
            printf("Error: Malformed agent entry - missing method name in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        
        // Copy method name with length check
        if (strlen(token) >= sizeof(method_name)) {
            printf("Error: Method name too long in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        strncpy(method_name, token, sizeof(method_name) - 1);
        method_name[sizeof(method_name) - 1] = '\0';  // Ensure null-termination
        
        // Get the method version
        token = strtok_r(NULL, " \t\n", &next_token);
        if (token == NULL) {
            printf("Error: Malformed agent entry - missing method version in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        
        // Copy method version with length check
        if (strlen(token) >= sizeof(method_version)) {
            printf("Error: Method version too long in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        strncpy(method_version, token, sizeof(method_version) - 1);
        method_version[sizeof(method_version) - 1] = '\0';  // Ensure null-termination
        
        // Create the agent
        agent_id_t new_id = ar_agent_create(method_name, method_version, NULL);
        if (new_id == 0) {
            printf("Error: Could not recreate agent %lld\n", id);
            continue;
        }
        // Note: The ar_agent_create function takes ownership of any context passed
        
        // Update the assigned ID to match the stored one
        for (int j = 0; j < MAX_AGENTS; j++) {
            if (g_own_agents[j].is_active && g_own_agents[j].id == new_id) {
                g_own_agents[j].id = id;
                
                // Read memory map line for count
                char mem_line[256];
                if (!ar_agency_safe_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                    printf("Error: Failed to read memory count\n");
                    break;
                }
                
                int mem_count = 0;
                // Parse memory count using strtol instead of sscanf
                char *mem_count_endptr = NULL;
                mem_count = (int)strtol(mem_line, &mem_count_endptr, 10);
                
                // Check for conversion errors or invalid values
                if (mem_count_endptr == mem_line || (*mem_count_endptr != '\0' && *mem_count_endptr != '\n') || mem_count < 0) {
                    printf("Error: Could not read memory count\n");
                    break;
                }
                
                for (int k = 0; k < mem_count; k++) {
                    char key[256] = {0};
                    char type[32] = {0};
                    
                    // Read key and type line
                    if (!ar_agency_safe_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                        printf("Error: Failed to read memory key/type\n");
                        break;
                    }
                    
                    // Parse memory key and type manually using strtok_r
                    char *mem_token, *mem_next_token = NULL;
                    
                    // Get the key
                    mem_token = strtok_r(mem_line, " \t\n", &mem_next_token);
                    if (mem_token == NULL) {
                        printf("Error: Malformed memory entry - missing key in %s\n", AGENCY_FILE_NAME);
                        break;
                    }
                    
                    // Copy key with length check
                    if (strlen(mem_token) >= sizeof(key)) {
                        printf("Error: Memory key too long in %s\n", AGENCY_FILE_NAME);
                        break;
                    }
                    strncpy(key, mem_token, sizeof(key) - 1);
                    key[sizeof(key) - 1] = '\0';  // Ensure null-termination
                    
                    // Get the type
                    mem_token = strtok_r(NULL, " \t\n", &mem_next_token);
                    if (mem_token == NULL) {
                        printf("Error: Malformed memory entry - missing type in %s\n", AGENCY_FILE_NAME);
                        break;
                    }
                    
                    // Copy type with length check
                    if (strlen(mem_token) >= sizeof(type)) {
                        printf("Error: Memory type too long in %s\n", AGENCY_FILE_NAME);
                        break;
                    }
                    strncpy(type, mem_token, sizeof(type) - 1);
                    type[sizeof(type) - 1] = '\0';  // Ensure null-termination
                    
                    data_t *own_value = NULL; // Will be an owned value after creation
                    if (strcmp(type, "int") == 0) {
                        // Read value line
                        if (!ar_agency_safe_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                            printf("Error: Failed to read int value\n");
                            break;
                        }
                        
                        int int_value;
                        // Parse int value using strtol instead of sscanf
                        char *int_value_endptr = NULL;
                        int_value = (int)strtol(mem_line, &int_value_endptr, 10);
                        
                        // Check for conversion errors
                        if (int_value_endptr == mem_line || (*int_value_endptr != '\0' && *int_value_endptr != '\n')) {
                            printf("Error: Could not read int value\n");
                            break;
                        }
                        own_value = ar_data_create_integer(int_value);
                    } else if (strcmp(type, "double") == 0) {
                        // Read value line
                        if (!ar_agency_safe_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                            printf("Error: Failed to read double value\n");
                            break;
                        }
                        
                        double double_value;
                        // Parse double value using strtod instead of sscanf
                        char *double_value_endptr = NULL;
                        double_value = strtod(mem_line, &double_value_endptr);
                        
                        // Check for conversion errors
                        if (double_value_endptr == mem_line || (*double_value_endptr != '\0' && *double_value_endptr != '\n')) {
                            printf("Error: Could not read double value\n");
                            break;
                        }
                        own_value = ar_data_create_double(double_value);
                    } else if (strcmp(type, "string") == 0) {
                        // Read value line
                        if (!ar_agency_safe_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                            printf("Error: Failed to read string value\n");
                            break;
                        }
                        
                        // Remove trailing newline if present
                        size_t len = strlen(mem_line);
                        if (len > 0 && mem_line[len-1] == '\n') {
                            mem_line[len-1] = '\0';
                        }
                        
                        own_value = ar_data_create_string(mem_line);
                    } else {
                        // Skip unknown type by reading next line
                        if (!ar_agency_safe_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                            printf("Error: Could not skip unknown type\n");
                            break;
                        }
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

int ar_agency_update_agent_methods(const method_t *ref_old_method, const method_t *ref_new_method) {
    if (!g_is_initialized || !ref_old_method || !ref_new_method) {
        return 0;
    }
    
    // Verify that the methods are compatible (same major version)
    if (!ar_semver_are_compatible(
            ar_method_get_version(ref_old_method),
            ar_method_get_version(ref_new_method))) {
        printf("Warning: Cannot update agents to incompatible method version\n");
        return 0;
    }
    
    // Get method names for verification
    const char *old_name = ar_method_get_name(ref_old_method);
    const char *new_name = ar_method_get_name(ref_new_method);
    if (strcmp(old_name, new_name) != 0) {
        printf("Warning: Cannot update agents to a different method name\n");
        return 0;
    }
    
    // Get version strings for logging
    const char *old_version = ar_method_get_version(ref_old_method);
    const char *new_version = ar_method_get_version(ref_new_method);
    
    // Track how many agents we update
    int update_count = 0;
    
    // Prepare sleep and wake messages
    data_t *own_sleep_message = ar_data_create_string("__sleep__");
    if (!own_sleep_message) {
        return 0;
    }
    
    data_t *own_wake_message = ar_data_create_string("__wake__");
    if (!own_wake_message) {
        ar_data_destroy(own_sleep_message);
        return 0;
    }
    
    // Find all agents using the old method
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].ref_method == ref_old_method) {
            // Get the agent ID for sending messages
            agent_id_t agent_id = g_own_agents[i].id;
            
            // Step 1: Send sleep message
            printf("Updating agent %lld from method %s version %s to version %s\n",
                   agent_id, old_name, old_version, new_version);
                   
            // Send a copy of the sleep message
            data_t *own_sleep_copy = ar_data_create_string("__sleep__");
            if (own_sleep_copy) {
                ar_agent_send(agent_id, own_sleep_copy);
                own_sleep_copy = NULL; // Ownership transferred to agent's queue
            }
            
            // Step 2: Process the sleep message
            // This happens elsewhere in the system during normal processing
            
            // Step 3: Update the method reference
            g_own_agents[i].ref_method = ref_new_method;
            
            // Step 4: Send wake message
            data_t *own_wake_copy = ar_data_create_string("__wake__");
            if (own_wake_copy) {
                ar_agent_send(agent_id, own_wake_copy);
                own_wake_copy = NULL; // Ownership transferred to agent's queue
            }
            
            update_count++;
        }
    }
    
    // Clean up
    ar_data_destroy(own_sleep_message);
    ar_data_destroy(own_wake_message);
    
    return update_count;
}

/* End of implementation */

