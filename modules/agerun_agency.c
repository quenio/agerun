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
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

/* Constants */

/* Global State */
static agent_t g_own_agents[MAX_AGENTS]; // Owned by the agency module
static agent_id_t g_next_agent_id = 1;
static bool g_is_initialized = false;

/**
 * Helper function for secure file reading with bounds checking
 *
 * @param fp Pointer to an open file
 * @param buffer Buffer to store the read line
 * @param buffer_size Size of the buffer
 * @param filename Name of the file being read (for error messages)
 * @return true if a line was read successfully, false otherwise
 */
static bool ar_agency_safe_read_line(FILE *fp, char *buffer, int buffer_size, const char *filename) {
    // Validate input parameters
    if (!fp || !buffer || buffer_size <= 0 || !filename) {
        fprintf(stderr, "Error: Invalid parameters for safe_read_line\n");
        return false;
    }
    
    // Initialize buffer with empty string for safety
    buffer[0] = '\0';
    
    // First check for EOF
    if (feof(fp)) {
        fprintf(stderr, "Error: Unexpected end of file in %s\n", filename);
        return false;
    }
    
    // Check for errors and clear them if present
    clearerr(fp);
    
    // Read character by character with bounds checking
    int i = 0;
    int c;
    
    // Ensure we leave room for null terminator and optional newline
    const int max_chars = buffer_size - 2;
    
    // Read with strict bounds checking
    while (i < max_chars) {
        c = fgetc(fp);
        
        // Check for EOF or newline
        if (c == EOF || c == '\n') {
            break;
        }
        
        // Check for control characters (except tab, which is allowed)
        if (iscntrl(c) && c != '\t') {
            // Skip control characters for security, but don't fail
            continue;
        }
        
        // Store valid character
        buffer[i++] = (char)c;
    }
    
    // Ensure null termination
    buffer[i] = '\0';
    
    // Handle newline if we found one (preserve it)
    if (c == '\n') {
        // Only add the newline if we have room
        if (i < max_chars) {
            buffer[i] = '\n';
            buffer[i+1] = '\0';
        }
    }
    
    // Check for file errors
    if (ferror(fp)) {
        fprintf(stderr, "Error: Failed to read file %s\n", filename);
        clearerr(fp);
        return false;
    }
    
    // Check if we hit EOF with no content
    if (i == 0 && c == EOF) {
        fprintf(stderr, "Error: Unexpected end of file in %s\n", filename);
        return false;
    }
    
    // Check if line was truncated due to buffer size
    if (i >= max_chars && c != '\n' && c != EOF) {
        // Continue reading until end of line or EOF to maintain file position
        // but don't store these characters
        while ((c = fgetc(fp)) != EOF && c != '\n') {
            // Just consume the characters
        }
        fprintf(stderr, "Warning: Line truncated in %s (buffer size: %d)\n", filename, buffer_size);
        // We still return true as we did read data successfully
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
    
    // Create a temporary file first, then rename it to avoid data corruption
    const char *temp_filename = AGENCY_FILE_NAME ".tmp";
    
    // Use restricted mode "w" to ensure we only create/write, not execute
    FILE *fp = fopen(temp_filename, "w");
    if (!fp) {
        // Use fprintf to stderr instead of printf for error reporting
        fprintf(stderr, "Error: Could not open %s for writing (error: %s)\n", 
                temp_filename, strerror(errno));
        return false;
    }
    
    // Set secure file permissions for the file (restrict to owner read/write)
    #ifdef _WIN32
    _chmod(temp_filename, _S_IREAD | _S_IWRITE);
    #else
    chmod(temp_filename, S_IRUSR | S_IWUSR);
    #endif
    
    // Count how many active agents we have
    int count = 0;
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].ref_method != NULL) {
            count++;
        }
    }
    
    // Use snprintf to safely format the output
    char buffer[128];
    int written = snprintf(buffer, sizeof(buffer), "%d\n", count);
    if (written < 0 || written >= (int)sizeof(buffer)) {
        fprintf(stderr, "Error: Buffer too small for count in %s\n", temp_filename);
        fclose(fp);
        remove(temp_filename);
        return false;
    }
    
    // Write the count to the file with error checking
    if (fputs(buffer, fp) == EOF) {
        fprintf(stderr, "Error: Failed to write count to %s\n", temp_filename);
        fclose(fp);
        remove(temp_filename);
        return false;
    }
    
    // Save basic agent info
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].ref_method != NULL) {
            // Get method name and version from the method reference
            const char *method_name = ar_method_get_name(g_own_agents[i].ref_method);
            const char *method_version = ar_method_get_version(g_own_agents[i].ref_method);
            
            if (!method_name || !method_version) {
                fprintf(stderr, "Error: Invalid method reference data for agent %lld\n", 
                        g_own_agents[i].id);
                fclose(fp);
                remove(temp_filename);
                return false;
            }
            
            // Use snprintf to safely format agent data
            written = snprintf(buffer, sizeof(buffer), "%lld %s %s\n", 
                              g_own_agents[i].id, method_name, method_version);
            if (written < 0 || written >= (int)sizeof(buffer)) {
                fprintf(stderr, "Error: Buffer too small for agent data in %s\n", temp_filename);
                fclose(fp);
                remove(temp_filename);
                return false;
            }
            
            // Write the agent data to the file with error checking
            if (fputs(buffer, fp) == EOF) {
                fprintf(stderr, "Error: Failed to write agent data to %s\n", temp_filename);
                fclose(fp);
                remove(temp_filename);
                return false;
            }
            
            // Save memory map placeholder (0 items for now)
            if (fputs("0\n", fp) == EOF) {
                fprintf(stderr, "Error: Failed to write memory map count to %s\n", temp_filename);
                fclose(fp);
                remove(temp_filename);
                return false;
            }
        }
    }
    
    // Flush and close the file
    if (fflush(fp) != 0) {
        fprintf(stderr, "Error: Failed to flush data to %s\n", temp_filename);
        fclose(fp);
        remove(temp_filename);
        return false;
    }
    
    fclose(fp);
    
    // Rename the temporary file to the permanent file
    if (rename(temp_filename, AGENCY_FILE_NAME) != 0) {
        fprintf(stderr, "Error: Failed to rename %s to %s\n", temp_filename, AGENCY_FILE_NAME);
        remove(temp_filename);
        return false;
    }
    
    return true;
}

bool ar_agency_load_agents(void) {
    if (!g_is_initialized) {
        return false;
    }
    
    // Securely open the file with error checking
    FILE *fp = fopen(AGENCY_FILE_NAME, "r");
    if (!fp) {
        // Not an error, might be first run
        return true;
    }
    
    // Verify file permissions (should be readable only by owner)
    struct stat file_stat;
    if (stat(AGENCY_FILE_NAME, &file_stat) == 0) {
        // Check if file has secure permissions (owner read/write only)
        #ifdef _WIN32
        // Windows doesn't expose the same permission bits, but we can check if it's readonly
        if (file_stat.st_mode & _S_IREAD && !(file_stat.st_mode & _S_IWRITE)) {
            fprintf(stderr, "Warning: Agency file is read-only\n");
        }
        #else
        // On Unix-like systems, check if file is accessible by others
        if (file_stat.st_mode & (S_IRWXG | S_IRWXO)) {
            fprintf(stderr, "Warning: Agency file has insecure permissions\n");
            // We continue anyway, but we've warned the user
        }
        #endif
    }
    
    // Read the first line to get agent count
    char line[256] = {0}; // Initialize to all zeros
    
    if (!ar_agency_safe_read_line(fp, line, (int)sizeof(line), AGENCY_FILE_NAME)) {
        fprintf(stderr, "Error: Failed to read agent count from %s\n", AGENCY_FILE_NAME);
        fclose(fp);
        return false;
    }
    
    // Parse agent count using secure strtol with error checking
    int count = 0;
    char *line_endptr = NULL;
    errno = 0; // Reset errno to check for conversion errors
    count = (int)strtol(line, &line_endptr, 10);
    
    // Validate conversion success and range
    if (errno != 0 || line_endptr == line || (*line_endptr != '\0' && *line_endptr != '\n') || 
        count < 0 || count > MAX_AGENTS) {
        fprintf(stderr, "Error: Invalid agent count in %s\n", AGENCY_FILE_NAME);
        fclose(fp);
        return false;
    }
    
    // Process each agent
    for (int i = 0; i < count; i++) {
        agent_id_t id = 0;
        char method_name[256] = {0}; // Initialized to zeros for security
        char method_version[64] = {0}; // Initialized to zeros for security
        
        // Read the agent line
        if (!ar_agency_safe_read_line(fp, line, (int)sizeof(line), AGENCY_FILE_NAME)) {
            fprintf(stderr, "Error: Failed to read agent entry %d from %s\n", i+1, AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        
        // Parse line manually for better security than sscanf
        char *token = NULL;
        char *next_token = NULL;
        
        // Get the ID
        token = strtok_r(line, " \t\n", &next_token);
        if (token == NULL) {
            fprintf(stderr, "Error: Malformed agent entry - missing ID in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        
        // Convert ID from string to agent_id_t (long long) with security checks
        char *id_endptr = NULL;
        errno = 0; // Reset errno to check for conversion errors
        id = strtoll(token, &id_endptr, 10);
        
        // Validate conversion
        if (errno != 0 || id_endptr == token || *id_endptr != '\0' || id <= 0) {
            fprintf(stderr, "Error: Invalid agent ID in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        
        // Get the method name
        token = strtok_r(NULL, " \t\n", &next_token);
        if (token == NULL) {
            fprintf(stderr, "Error: Malformed agent entry - missing method name in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        
        // Copy method name with secure length check
        size_t token_len = strlen(token);
        if (token_len >= sizeof(method_name)) {
            fprintf(stderr, "Error: Method name too long in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        
        // Use memcpy with explicit null termination for safer copy
        memcpy(method_name, token, token_len);
        method_name[token_len] = '\0';  // Ensure null-termination
        
        // Get the method version
        token = strtok_r(NULL, " \t\n", &next_token);
        if (token == NULL) {
            fprintf(stderr, "Error: Malformed agent entry - missing method version in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        
        // Copy method version with secure length check
        token_len = strlen(token);
        if (token_len >= sizeof(method_version)) {
            fprintf(stderr, "Error: Method version too long in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        
        // Use memcpy with explicit null termination for safer copy
        memcpy(method_version, token, token_len);
        method_version[token_len] = '\0';  // Ensure null-termination
        
        // Validate the method name and version - basic sanity check
        if (strlen(method_name) == 0 || strlen(method_version) == 0) {
            fprintf(stderr, "Error: Empty method name or version in %s\n", AGENCY_FILE_NAME);
            fclose(fp);
            return false;
        }
        
        // Create the agent
        agent_id_t new_id = ar_agent_create(method_name, method_version, NULL);
        if (new_id == 0) {
            fprintf(stderr, "Error: Could not recreate agent %lld\n", id);
            continue; // Skip this agent but continue processing
        }
        
        // Update the assigned ID to match the stored one
        for (int j = 0; j < MAX_AGENTS; j++) {
            if (g_own_agents[j].is_active && g_own_agents[j].id == new_id) {
                g_own_agents[j].id = id;
                
                // Read memory map line for count
                char mem_line[256] = {0};
                if (!ar_agency_safe_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                    fprintf(stderr, "Error: Failed to read memory count for agent %lld\n", id);
                    break;
                }
                
                // Parse memory count securely
                int mem_count = 0;
                char *mem_count_endptr = NULL;
                errno = 0; // Reset errno to check for conversion errors
                mem_count = (int)strtol(mem_line, &mem_count_endptr, 10);
                
                // Validate conversion and range
                if (errno != 0 || mem_count_endptr == mem_line || 
                    (*mem_count_endptr != '\0' && *mem_count_endptr != '\n') || 
                    mem_count < 0 || mem_count > 1000) { // Adding reasonable upper limit
                    fprintf(stderr, "Error: Invalid memory count for agent %lld\n", id);
                    break;
                }
                
                // Process each memory item
                for (int k = 0; k < mem_count; k++) {
                    char key[256] = {0};
                    char type[32] = {0};
                    
                    // Read key and type line
                    if (!ar_agency_safe_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                        fprintf(stderr, "Error: Failed to read memory key/type for agent %lld\n", id);
                        break;
                    }
                    
                    // Parse key and type
                    char *mem_token = NULL;
                    char *mem_next_token = NULL;
                    
                    // Get the key
                    mem_token = strtok_r(mem_line, " \t\n", &mem_next_token);
                    if (mem_token == NULL) {
                        fprintf(stderr, "Error: Malformed memory entry - missing key for agent %lld\n", id);
                        break;
                    }
                    
                    // Copy key securely
                    token_len = strlen(mem_token);
                    if (token_len >= sizeof(key)) {
                        fprintf(stderr, "Error: Memory key too long for agent %lld\n", id);
                        break;
                    }
                    
                    // Use memcpy with explicit null termination for safer copy
                    memcpy(key, mem_token, token_len);
                    key[token_len] = '\0';  // Ensure null-termination
                    
                    // Get the type
                    mem_token = strtok_r(NULL, " \t\n", &mem_next_token);
                    if (mem_token == NULL) {
                        fprintf(stderr, "Error: Malformed memory entry - missing type for agent %lld\n", id);
                        break;
                    }
                    
                    // Copy type securely
                    token_len = strlen(mem_token);
                    if (token_len >= sizeof(type)) {
                        fprintf(stderr, "Error: Memory type too long for agent %lld\n", id);
                        break;
                    }
                    
                    // Use memcpy with explicit null termination for safer copy
                    memcpy(type, mem_token, token_len);
                    type[token_len] = '\0';  // Ensure null-termination
                    
                    // Process based on type
                    data_t *own_value = NULL; // Will be an owned value after creation
                    
                    if (strcmp(type, "int") == 0) {
                        // Read value line
                        if (!ar_agency_safe_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                            fprintf(stderr, "Error: Failed to read int value for agent %lld\n", id);
                            break;
                        }
                        
                        // Parse int value securely
                        int int_value;
                        char *int_value_endptr = NULL;
                        errno = 0; // Reset errno to check for conversion errors
                        int_value = (int)strtol(mem_line, &int_value_endptr, 10);
                        
                        // Validate conversion
                        if (errno != 0 || int_value_endptr == mem_line || 
                            (*int_value_endptr != '\0' && *int_value_endptr != '\n')) {
                            fprintf(stderr, "Error: Invalid int value for agent %lld\n", id);
                            break;
                        }
                        
                        own_value = ar_data_create_integer(int_value);
                    } else if (strcmp(type, "double") == 0) {
                        // Read value line
                        if (!ar_agency_safe_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                            fprintf(stderr, "Error: Failed to read double value for agent %lld\n", id);
                            break;
                        }
                        
                        // Parse double value securely
                        double double_value;
                        char *double_value_endptr = NULL;
                        errno = 0; // Reset errno to check for conversion errors
                        double_value = strtod(mem_line, &double_value_endptr);
                        
                        // Validate conversion
                        if (errno != 0 || double_value_endptr == mem_line || 
                            (*double_value_endptr != '\0' && *double_value_endptr != '\n')) {
                            fprintf(stderr, "Error: Invalid double value for agent %lld\n", id);
                            break;
                        }
                        
                        own_value = ar_data_create_double(double_value);
                    } else if (strcmp(type, "string") == 0) {
                        // Read value line
                        if (!ar_agency_safe_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                            fprintf(stderr, "Error: Failed to read string value for agent %lld\n", id);
                            break;
                        }
                        
                        // Remove trailing newline if present
                        size_t len = strlen(mem_line);
                        if (len > 0 && mem_line[len-1] == '\n') {
                            mem_line[len-1] = '\0';
                        }
                        
                        // Create string data (makes a copy)
                        own_value = ar_data_create_string(mem_line);
                    } else {
                        // Skip unknown type by reading next line
                        fprintf(stderr, "Warning: Unknown memory type '%s' for agent %lld\n", type, id);
                        if (!ar_agency_safe_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                            fprintf(stderr, "Error: Could not skip unknown type for agent %lld\n", id);
                            break;
                        }
                        continue;
                    }
                    
                    // Add the value to the agent's memory map
                    if (own_value) {
                        if (!g_own_agents[j].own_memory) {
                            fprintf(stderr, "Error: Agent %lld has no memory map\n", id);
                            ar_data_destroy(own_value);
                            break;
                        }
                        
                        ar_data_set_map_data(g_own_agents[j].own_memory, key, own_value);
                        // Note: Data ownership is transferred, so we don't free value here
                        own_value = NULL; // Mark as transferred
                    } else {
                        fprintf(stderr, "Error: Failed to create value for agent %lld\n", id);
                        break;
                    }
                }
                
                break; // Found the agent, no need to continue searching
            }
        }
        
        // Update next_agent_id if needed to ensure no ID collision
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

