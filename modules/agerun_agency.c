/* Agerun Agency Implementation */
#include "agerun_agency.h"
#include "agerun_agent.h"
#include "agerun_map.h"
#include "agerun_data.h"
#include "agerun_method.h"
#include "agerun_semver.h"
#include "agerun_io.h" /* Include the I/O utilities */

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

// We now use ar_io_read_line directly from the agerun_io module

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

// Context structure for agency saving
typedef struct {
    const char *filename;
} agency_save_context_t;

// Writer function for agency data
static bool agency_write_function(FILE *fp, void *context) {
    agency_save_context_t *ctx = (agency_save_context_t *)context;
    if (!ctx) {
        ar_io_error("Invalid context for agency_write_function");
        return false;
    }

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
        ar_io_error("Buffer too small for count in %s", ctx->filename);
        return false;
    }

    // Write the count to the file with error checking
    if (fputs(buffer, fp) == EOF) {
        ar_io_error("Failed to write count to %s", ctx->filename);
        return false;
    }

    // Save basic agent info
    for (int i = 0; i < MAX_AGENTS; i++) {
        if (g_own_agents[i].is_active && g_own_agents[i].ref_method != NULL) {
            // Get method name and version from the method reference
            const char *method_name = ar_method_get_name(g_own_agents[i].ref_method);
            const char *method_version = ar_method_get_version(g_own_agents[i].ref_method);

            if (!method_name || !method_version) {
                ar_io_error("Invalid method reference data for agent %lld", g_own_agents[i].id);
                return false;
            }

            // Use snprintf to safely format agent data
            written = snprintf(buffer, sizeof(buffer), "%lld %s %s\n",
                              g_own_agents[i].id, method_name, method_version);
            if (written < 0 || written >= (int)sizeof(buffer)) {
                ar_io_error("Buffer too small for agent data in %s", ctx->filename);
                return false;
            }

            // Write the agent data to the file with error checking
            if (fputs(buffer, fp) == EOF) {
                ar_io_error("Failed to write agent data to %s", ctx->filename);
                return false;
            }

            // Save memory map placeholder (0 items for now)
            if (fputs("0\n", fp) == EOF) {
                ar_io_error("Failed to write memory map count to %s", ctx->filename);
                return false;
            }
        }
    }

    // Success
    return true;
}

bool ar_agency_save_agents(void) {
    if (!g_is_initialized) {
        return false;
    }

    // Set up context for writer function
    agency_save_context_t context = {
        .filename = AGENCY_FILE_NAME
    };

    // Use the safe file writing utility
    file_result_t result = ar_io_write_file(AGENCY_FILE_NAME, agency_write_function, &context);
    if (result != FILE_SUCCESS) {
        ar_io_error("Failed to save agents file: %s", ar_io_error_message(result));
        return false;
    }

    return true;
}

// Data structure to hold processed agent info during loading
typedef struct {
    agent_id_t id;
    char method_name[256];
    char method_version[64];
} agency_load_agent_info_t;

/**
 * Validates the format of an agency file to ensure it can be loaded
 * Provides details about any issues found
 *
 * @param filename Path to the agency file
 * @param error_message Buffer to store error message if validation fails
 * @param error_size Size of the error message buffer
 * @return true if file is valid, false if it has formatting issues
 */
static bool ar_agency_validate_file(const char *filename, char *error_message, size_t error_size) {
    FILE *fp;
    file_result_t result = ar_io_open_file(filename, "r", &fp);

    if (result == FILE_ERROR_NOT_FOUND) {
        snprintf(error_message, error_size, "Agency file %s not found", filename);
        return false;
    } else if (result != FILE_SUCCESS) {
        snprintf(error_message, error_size, "Failed to open agency file: %s",
                ar_io_error_message(result));
        return false;
    }

    // Read and validate agent count
    char line[256] = {0};
    if (!ar_io_read_line(fp, line, (int)sizeof(line), filename)) {
        snprintf(error_message, error_size, "Failed to read agent count from %s", filename);
        ar_io_close_file(fp, filename);
        return false;
    }

    int count = 0;
    char *line_endptr = NULL;
    errno = 0;
    count = (int)strtol(line, &line_endptr, 10);

    if (errno != 0 || line_endptr == line || (*line_endptr != '\0' && *line_endptr != '\n') ||
        count < 0 || count > MAX_AGENTS) {
        snprintf(error_message, error_size, "Invalid agent count in %s", filename);
        ar_io_close_file(fp, filename);
        return false;
    }

    // Validate each agent entry
    for (int i = 0; i < count; i++) {
        // Read and validate agent line
        if (!ar_io_read_line(fp, line, (int)sizeof(line), filename)) {
            snprintf(error_message, error_size, "Failed to read agent entry %d from %s", i+1, filename);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Count tokens - should be exactly 3 (id, method_name, method_version)
        char *token = NULL;
        char *next_token = NULL;
        int tokens = 0;
        char *saveptr = NULL;
        char line_copy[256];

        // Make a copy of the line since strtok_r modifies it
        strncpy(line_copy, line, sizeof(line_copy) - 1);
        line_copy[sizeof(line_copy) - 1] = '\0';

        // Count tokens
        token = strtok_r(line_copy, " \t\n", &saveptr);
        while (token != NULL) {
            tokens++;
            token = strtok_r(NULL, " \t\n", &saveptr);
        }

        if (tokens != 3) {
            snprintf(error_message, error_size,
                    "Malformed agent entry for agent %d in %s: expected 3 fields, found %d",
                    i+1, filename, tokens);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Validate ID, method name, and version format
        token = strtok_r(line, " \t\n", &next_token);
        if (token == NULL) {
            snprintf(error_message, error_size,
                    "Malformed agent entry - missing ID in %s for agent %d", filename, i+1);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Validate ID is a number
        char *id_endptr = NULL;
        errno = 0;
        agent_id_t id = strtoll(token, &id_endptr, 10);
        if (errno != 0 || id_endptr == token || *id_endptr != '\0' || id <= 0) {
            snprintf(error_message, error_size,
                    "Invalid agent ID '%s' in %s for agent %d", token, filename, i+1);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Validate method name exists
        token = strtok_r(NULL, " \t\n", &next_token);
        if (token == NULL) {
            snprintf(error_message, error_size,
                    "Malformed agent entry - missing method name in %s for agent %d", filename, i+1);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Validate method name is not empty
        if (strlen(token) == 0) {
            snprintf(error_message, error_size,
                    "Empty method name in %s for agent %d", filename, i+1);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Validate method version exists
        token = strtok_r(NULL, " \t\n", &next_token);
        if (token == NULL) {
            snprintf(error_message, error_size,
                    "Malformed agent entry - missing method version in %s for agent %d", filename, i+1);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Validate method version is not empty
        if (strlen(token) == 0) {
            snprintf(error_message, error_size,
                    "Empty method version in %s for agent %d", filename, i+1);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Read memory map count line
        if (!ar_io_read_line(fp, line, (int)sizeof(line), filename)) {
            snprintf(error_message, error_size,
                    "Failed to read memory count for agent %d in %s", i+1, filename);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Validate memory count is a number
        char *mem_count_endptr = NULL;
        errno = 0;
        int mem_count = (int)strtol(line, &mem_count_endptr, 10);
        if (errno != 0 || mem_count_endptr == line ||
            (*mem_count_endptr != '\0' && *mem_count_endptr != '\n') ||
            mem_count < 0 || mem_count > 1000) {
            snprintf(error_message, error_size,
                    "Invalid memory count '%s' for agent %d in %s", line, i+1, filename);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Skip memory entries (we don't validate them in this pass)
        for (int j = 0; j < mem_count; j++) {
            // Skip key and type line
            if (!ar_io_read_line(fp, line, (int)sizeof(line), filename)) {
                snprintf(error_message, error_size,
                        "Failed to read memory key/type for agent %d in %s", i+1, filename);
                ar_io_close_file(fp, filename);
                return false;
            }

            // Parse type to know how many more lines to skip
            token = NULL;
            next_token = NULL;

            // Skip key
            token = strtok_r(line, " \t\n", &next_token);
            if (token == NULL) {
                snprintf(error_message, error_size,
                        "Malformed memory entry - missing key for agent %d in %s", i+1, filename);
                ar_io_close_file(fp, filename);
                return false;
            }

            // Get type
            token = strtok_r(NULL, " \t\n", &next_token);
            if (token == NULL) {
                snprintf(error_message, error_size,
                        "Malformed memory entry - missing type for agent %d in %s", i+1, filename);
                ar_io_close_file(fp, filename);
                return false;
            }

            // Skip value line (for all known types)
            if (strcmp(token, "int") == 0 || strcmp(token, "double") == 0 ||
                strcmp(token, "string") == 0 || strcmp(token, "unknown") == 0) {
                if (!ar_io_read_line(fp, line, (int)sizeof(line), filename)) {
                    snprintf(error_message, error_size,
                            "Failed to read memory value for agent %d in %s", i+1, filename);
                    ar_io_close_file(fp, filename);
                    return false;
                }
            } else {
                snprintf(error_message, error_size,
                        "Unknown memory type '%s' for agent %d in %s", token, i+1, filename);
                ar_io_close_file(fp, filename);
                return false;
            }
        }
    }

    ar_io_close_file(fp, filename);
    return true;
}

bool ar_agency_load_agents(void) {
    if (!g_is_initialized) {
        return false;
    }

    // First validate the agency file format
    char error_message[512];
    if (!ar_agency_validate_file(AGENCY_FILE_NAME, error_message, sizeof(error_message))) {
        if (strstr(error_message, "not found") != NULL) {
            // Not an error, might be first run
            return true;
        }

        // File exists but has errors
        ar_io_error("Agency file validation failed: %s", error_message);

        // Create a backup and remove the corrupted file
        ar_io_warning("Creating backup of corrupted agency file");
        ar_io_create_backup(AGENCY_FILE_NAME);
        remove(AGENCY_FILE_NAME);

        return true; // Return success but with empty state
    }

    // Securely open the file with proper error handling (now that we know it's valid)
    FILE *fp;
    file_result_t result = ar_io_open_file(AGENCY_FILE_NAME, "r", &fp);

    if (result != FILE_SUCCESS) {
        ar_io_error("Failed to open agency file: %s", ar_io_error_message(result));
        return false;
    }

    // Verify file permissions (should be readable only by owner)
    result = ar_io_set_secure_permissions(AGENCY_FILE_NAME);
    if (result != FILE_SUCCESS) {
        ar_io_warning("Failed to set secure permissions on agency file: %s",
                   ar_io_error_message(result));
        // Continue anyway with a warning
    }

    // Read the first line to get agent count
    char line[256] = {0}; // Initialize to all zeros

    if (!ar_io_read_line(fp, line, (int)sizeof(line), AGENCY_FILE_NAME)) {
        ar_io_error("Failed to read agent count from %s", AGENCY_FILE_NAME);
        ar_io_close_file(fp, AGENCY_FILE_NAME);
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
        ar_io_error("Invalid agent count in %s", AGENCY_FILE_NAME);
        ar_io_close_file(fp, AGENCY_FILE_NAME);

        // File is corrupt, create a backup and potentially delete it
        ar_io_warning("Agency file appears to be corrupt. Creating backup and continuing with empty state.");
        ar_io_create_backup(AGENCY_FILE_NAME);
        remove(AGENCY_FILE_NAME);
        return true; // Return success but with empty state
    }

    // Check if count is 0, which is valid but no agents to process
    if (count == 0) {
        ar_io_close_file(fp, AGENCY_FILE_NAME);
        return true;
    }

    // First-pass: Read all agent basic info to validate the file structure
    // Calculate size with proper type to avoid sign conversion
    const size_t agent_info_size = (size_t)count * sizeof(agency_load_agent_info_t);
    agency_load_agent_info_t *agent_info = (agency_load_agent_info_t*)malloc(agent_info_size);
    if (!agent_info) {
        ar_io_error("Memory allocation failed for agent info");
        ar_io_close_file(fp, AGENCY_FILE_NAME);
        return false;
    }

    // Initialize the agent info array
    memset(agent_info, 0, agent_info_size);

    // Flag to track any validation errors
    bool validation_error = false;

    // First pass: validate basic agent info
    for (int i = 0; i < count; i++) {
        // Read the agent line
        if (!ar_io_read_line(fp, line, (int)sizeof(line), AGENCY_FILE_NAME)) {
            ar_io_error("Failed to read agent entry %d from %s", i+1, AGENCY_FILE_NAME);
            validation_error = true;
            break;
        }

        // Parse line manually for better security than sscanf
        char *token = NULL;
        char *next_token = NULL;

        // Get the ID
        token = strtok_r(line, " \t\n", &next_token);
        if (token == NULL) {
            ar_io_error("Malformed agent entry - missing ID in %s", AGENCY_FILE_NAME);
            validation_error = true;
            break;
        }

        // Convert ID from string to agent_id_t (long long) with security checks
        char *id_endptr = NULL;
        errno = 0; // Reset errno to check for conversion errors
        agent_info[i].id = strtoll(token, &id_endptr, 10);

        // Validate conversion
        if (errno != 0 || id_endptr == token || *id_endptr != '\0' || agent_info[i].id <= 0) {
            ar_io_error("Invalid agent ID in %s", AGENCY_FILE_NAME);
            validation_error = true;
            break;
        }

        // Get the method name
        token = strtok_r(NULL, " \t\n", &next_token);
        if (token == NULL) {
            ar_io_error("Malformed agent entry - missing method name in %s", AGENCY_FILE_NAME);
            validation_error = true;
            break;
        }

        // Copy method name with secure length check
        size_t token_len = strlen(token);
        if (token_len >= sizeof(agent_info[i].method_name)) {
            ar_io_error("Method name too long in %s", AGENCY_FILE_NAME);
            validation_error = true;
            break;
        }

        // Use memcpy with explicit null termination for safer copy
        memcpy(agent_info[i].method_name, token, token_len);
        agent_info[i].method_name[token_len] = '\0';  // Ensure null-termination

        // Get the method version
        token = strtok_r(NULL, " \t\n", &next_token);
        if (token == NULL) {
            ar_io_error("Malformed agent entry - missing method version in %s", AGENCY_FILE_NAME);
            validation_error = true;
            break;
        }

        // Copy method version with secure length check
        token_len = strlen(token);
        if (token_len >= sizeof(agent_info[i].method_version)) {
            ar_io_error("Method version too long in %s", AGENCY_FILE_NAME);
            validation_error = true;
            break;
        }

        // Use memcpy with explicit null termination for safer copy
        memcpy(agent_info[i].method_version, token, token_len);
        agent_info[i].method_version[token_len] = '\0';  // Ensure null-termination

        // Validate the method name and version - basic sanity check
        if (strlen(agent_info[i].method_name) == 0 || strlen(agent_info[i].method_version) == 0) {
            ar_io_error("Empty method name or version in %s", AGENCY_FILE_NAME);
            validation_error = true;
            break;
        }

        // Skip memory map placeholder (0 items for now) - validation only
        if (!ar_io_read_line(fp, line, (int)sizeof(line), AGENCY_FILE_NAME)) {
            ar_io_error("Failed to read memory map placeholder for agent %lld", agent_info[i].id);
            validation_error = true;
            break;
        }
    }

    // If we encountered any validation errors, handle gracefully
    if (validation_error) {
        free(agent_info);
        ar_io_close_file(fp, AGENCY_FILE_NAME);

        // File is corrupt, create a backup and potentially delete it
        ar_io_warning("Agency file appears to be corrupt. Creating backup and continuing with empty state.");
        ar_io_create_backup(AGENCY_FILE_NAME);
        remove(AGENCY_FILE_NAME);
        return true; // Return success but with empty state
    }

    // Reset file position to beginning for second pass
    rewind(fp);

    // Skip the agent count line since we already know it
    if (!ar_io_read_line(fp, line, (int)sizeof(line), AGENCY_FILE_NAME)) {
        ar_io_error("Failed to skip agent count line");
        free(agent_info);
        ar_io_close_file(fp, AGENCY_FILE_NAME);
        return false;
    }

    // Second pass: Actually create the agents now that we know the file structure is valid
    for (int i = 0; i < count; i++) {
        // Skip the agent info line since we already have it
        if (!ar_io_read_line(fp, line, (int)sizeof(line), AGENCY_FILE_NAME)) {
            ar_io_error("Failed to skip agent info line for agent %d", i+1);
            break; // Continue with agents processed so far
        }

        // Create the agent
        agent_id_t new_id = ar_agent_create(
            agent_info[i].method_name,
            agent_info[i].method_version,
            NULL);

        if (new_id == 0) {
            ar_io_error("Could not recreate agent %lld", agent_info[i].id);
            // Skip this agent but continue processing

            // Skip memory map placeholder
            if (!ar_io_read_line(fp, line, (int)sizeof(line), AGENCY_FILE_NAME)) {
                ar_io_error("Failed to skip memory map placeholder for failed agent creation");
                break;
            }

            continue;
        }

        // Update the assigned ID to match the stored one
        bool agent_found = false;
        for (int j = 0; j < MAX_AGENTS; j++) {
            if (g_own_agents[j].is_active && g_own_agents[j].id == new_id) {
                g_own_agents[j].id = agent_info[i].id;
                agent_found = true;

                // Read memory map line for count
                char mem_line[256] = {0};
                if (!ar_io_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                    ar_io_error("Failed to read memory count for agent %lld", agent_info[i].id);
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
                    ar_io_error("Invalid memory count for agent %lld", agent_info[i].id);
                    break;
                }

                // Process each memory item
                for (int k = 0; k < mem_count; k++) {
                    char key[256] = {0};
                    char type[32] = {0};

                    // Read key and type line
                    if (!ar_io_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                        ar_io_error("Failed to read memory key/type for agent %lld", agent_info[i].id);
                        break;
                    }

                    // Parse key and type
                    char *mem_token = NULL;
                    char *mem_next_token = NULL;

                    // Get the key
                    mem_token = strtok_r(mem_line, " \t\n", &mem_next_token);
                    if (mem_token == NULL) {
                        ar_io_error("Malformed memory entry - missing key for agent %lld", agent_info[i].id);
                        break;
                    }

                    // Copy key securely
                    size_t token_len = strlen(mem_token);
                    if (token_len >= sizeof(key)) {
                        ar_io_error("Memory key too long for agent %lld", agent_info[i].id);
                        break;
                    }

                    // Use memcpy with explicit null termination for safer copy
                    memcpy(key, mem_token, token_len);
                    key[token_len] = '\0';  // Ensure null-termination

                    // Get the type
                    mem_token = strtok_r(NULL, " \t\n", &mem_next_token);
                    if (mem_token == NULL) {
                        ar_io_error("Malformed memory entry - missing type for agent %lld", agent_info[i].id);
                        break;
                    }

                    // Copy type securely
                    token_len = strlen(mem_token);
                    if (token_len >= sizeof(type)) {
                        ar_io_error("Memory type too long for agent %lld", agent_info[i].id);
                        break;
                    }

                    // Use memcpy with explicit null termination for safer copy
                    memcpy(type, mem_token, token_len);
                    type[token_len] = '\0';  // Ensure null-termination

                    // Process based on type
                    data_t *own_value = NULL; // Will be an owned value after creation

                    if (strcmp(type, "int") == 0) {
                        // Read value line
                        if (!ar_io_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                            ar_io_error("Failed to read int value for agent %lld", agent_info[i].id);
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
                            ar_io_error("Invalid int value for agent %lld", agent_info[i].id);
                            break;
                        }

                        own_value = ar_data_create_integer(int_value);
                    } else if (strcmp(type, "double") == 0) {
                        // Read value line
                        if (!ar_io_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                            ar_io_error("Failed to read double value for agent %lld", agent_info[i].id);
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
                            ar_io_error("Invalid double value for agent %lld", agent_info[i].id);
                            break;
                        }

                        own_value = ar_data_create_double(double_value);
                    } else if (strcmp(type, "string") == 0) {
                        // Read value line
                        if (!ar_io_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                            ar_io_error("Failed to read string value for agent %lld", agent_info[i].id);
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
                        ar_io_warning("Unknown memory type '%s' for agent %lld", type, agent_info[i].id);
                        if (!ar_io_read_line(fp, mem_line, (int)sizeof(mem_line), AGENCY_FILE_NAME)) {
                            ar_io_error("Could not skip unknown type for agent %lld", agent_info[i].id);
                            break;
                        }
                        continue;
                    }

                    // Add the value to the agent's memory map
                    if (own_value) {
                        if (!g_own_agents[j].own_memory) {
                            ar_io_error("Agent %lld has no memory map", agent_info[i].id);
                            ar_data_destroy(own_value);
                            break;
                        }

                        ar_data_set_map_data(g_own_agents[j].own_memory, key, own_value);
                        // Note: Data ownership is transferred, so we don't free value here
                        own_value = NULL; // Mark as transferred
                    } else {
                        ar_io_error("Failed to create value for agent %lld", agent_info[i].id);
                        break;
                    }
                }

                break; // Found the agent, no need to continue searching
            }
        }

        // If agent was not found (this shouldn't happen normally), skip the memory map data
        if (!agent_found) {
            ar_io_error("Agent with ID %lld was created but not found in agents array", agent_info[i].id);
            // Skip memory map placeholder
            if (!ar_io_read_line(fp, line, (int)sizeof(line), AGENCY_FILE_NAME)) {
                ar_io_error("Failed to skip memory map placeholder for unfound agent");
                break;
            }
        }

        // Update next_agent_id if needed to ensure no ID collision
        if (agent_info[i].id >= g_next_agent_id) {
            g_next_agent_id = agent_info[i].id + 1;
        }
    }

    // Clean up resources
    free(agent_info);
    ar_io_close_file(fp, AGENCY_FILE_NAME);

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
        ar_io_warning("Cannot update agents to incompatible method version");
        return 0;
    }

    // Get method names for verification
    const char *old_name = ar_method_get_name(ref_old_method);
    const char *new_name = ar_method_get_name(ref_new_method);
    if (strcmp(old_name, new_name) != 0) {
        ar_io_warning("Cannot update agents to a different method name");
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
            ar_io_fprintf(stdout, "Updating agent %lld from method %s version %s to version %s\n",
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

