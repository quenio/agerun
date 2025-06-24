/**
 * @file agerun_agent_store.c
 * @brief Implementation of the agent store module
 */

#include "ar_agent_store.h"
#include "ar_agent.h"
#include "ar_agent_registry.h"
#include "ar_agency.h"
#include "ar_method.h"
#include "ar_data.h"
#include "ar_list.h"
#include "ar_io.h"
#include "ar_heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <inttypes.h>

/* Maximum reasonable limits */
#define MAX_STORE_AGENTS 10000
#define MAX_LINE_LENGTH 256
#define MAX_MEMORY_ITEMS 1000

/* Context structure for save operations */
typedef struct {
    const char *filename;
} store_save_context_t;

/* Helper function to get list of all active agent IDs */
static list_t* _get_active_agent_list(void) {
    list_t *own_list = ar__list__create();
    if (!own_list) {
        return NULL;
    }
    
    int64_t agent_id = ar__agency__get_first_agent();
    while (agent_id != 0) {
        data_t *own_id_data = ar__data__create_integer((int)agent_id);
        if (own_id_data) {
            ar__list__add_last(own_list, own_id_data);
        }
        agent_id = ar__agency__get_next_agent(agent_id);
    }
    
    return own_list;
}


/* Helper function to clean up agent list and items */
static void _cleanup_agent_list(list_t *own_agents, void **own_items, size_t count) {
    // Destroy data_t objects from the items array
    if (own_items) {
        for (size_t i = 0; i < count; i++) {
            if (own_items[i]) {
                ar__data__destroy((data_t*)own_items[i]);
            }
        }
        AR__HEAP__FREE(own_items);
    }
    
    // Now remove all items from the list (they've been destroyed)
    while (ar__list__count(own_agents) > 0) {
        ar__list__remove_first(own_agents);
    }
    
    ar__list__destroy(own_agents);
}

/* Writer function for saving agent data */
static bool _store_write_function(FILE *fp, void *context) {
    store_save_context_t *ctx = (store_save_context_t *)context;
    if (!ctx) {
        ar__io__error("Invalid context for _store_write_function");
        return false;
    }
    
    // Get list of all active agents
    list_t *own_agents = _get_active_agent_list();
    if (!own_agents) {
        ar__io__error("Failed to get agent list");
        return false;
    }
    
    // Get array of agent IDs
    size_t total_agents = ar__list__count(own_agents);
    if (total_agents == 0) {
        // No agents to save
        ar__list__destroy(own_agents);
        fprintf(fp, "0\n");
        return true;
    }
    
    void **own_items = ar__list__items(own_agents);
    if (!own_items) {
        ar__io__error("Failed to get agent items array");
        ar__list__destroy(own_agents);
        return false;
    }
    
    // Count agents with methods
    int count = 0;
    for (size_t i = 0; i < total_agents; i++) {
        data_t *ref_id_data = (data_t*)own_items[i];
        if (ref_id_data) {
            int64_t agent_id = ar__data__get_integer(ref_id_data);
            const method_t *ref_method = ar__agency__get_agent_method(agent_id);
            if (ref_method != NULL) {
                count++;
            }
        }
    }
    
    // Write the count
    char buffer[128];
    int written = snprintf(buffer, sizeof(buffer), "%d\n", count);
    if (written < 0 || written >= (int)sizeof(buffer)) {
        ar__io__error("Buffer too small for count in %s", ctx->filename);
        _cleanup_agent_list(own_agents, own_items, total_agents);
        return false;
    }
    
    if (fputs(buffer, fp) == EOF) {
        ar__io__error("Failed to write count to %s", ctx->filename);
        _cleanup_agent_list(own_agents, own_items, total_agents);
        return false;
    }
    
    // Save each agent's data
    for (size_t i = 0; i < total_agents; i++) {
        data_t *ref_id_data = (data_t*)own_items[i];
        if (!ref_id_data) {
            continue;
        }
        
        int64_t agent_id = ar__data__get_integer(ref_id_data);
        const method_t *ref_method = ar__agency__get_agent_method(agent_id);
        
        if (ref_method == NULL) {
            continue; // Skip agents without methods
        }
        
        const char *method_name = ar__method__get_name(ref_method);
        const char *method_version = ar__method__get_version(ref_method);
        
        if (!method_name || !method_version) {
            ar__io__error("Invalid method reference data for agent %" PRId64, agent_id);
            _cleanup_agent_list(own_agents, own_items, total_agents);
            return false;
        }
        
        // Write agent data
        written = snprintf(buffer, sizeof(buffer), "%" PRId64 " %s %s\n",
                          agent_id, method_name, method_version);
        if (written < 0 || written >= (int)sizeof(buffer)) {
            ar__io__error("Buffer too small for agent data in %s", ctx->filename);
            _cleanup_agent_list(own_agents, own_items, total_agents);
            return false;
        }
        
        if (fputs(buffer, fp) == EOF) {
            ar__io__error("Failed to write agent data to %s", ctx->filename);
            _cleanup_agent_list(own_agents, own_items, total_agents);
            return false;
        }
        
        // Get agent memory
        data_t *ref_memory = ar__agency__get_agent_mutable_memory(agent_id);
        
        if (!ref_memory || ar__data__get_type(ref_memory) != DATA_MAP) {
            // No memory or not a map - write 0 items
            if (fputs("0\n", fp) == EOF) {
                ar__io__error("Failed to write memory count to %s", ctx->filename);
                _cleanup_agent_list(own_agents, own_items, total_agents);
                return false;
            }
            continue;
        }
        
        // Get all keys from the memory map
        data_t *own_keys = ar__data__get_map_keys(ref_memory);
        if (!own_keys) {
            ar__io__error("Failed to get keys from agent memory");
            _cleanup_agent_list(own_agents, own_items, total_agents);
            return false;
        }
        
        // Write the number of memory items
        size_t memory_count = ar__data__list_count(own_keys);
        written = snprintf(buffer, sizeof(buffer), "%zu\n", memory_count);
        if (written < 0 || written >= (int)sizeof(buffer)) {
            ar__io__error("Buffer too small for memory count in %s", ctx->filename);
            ar__data__destroy(own_keys);
            _cleanup_agent_list(own_agents, own_items, total_agents);
            return false;
        }
        
        if (fputs(buffer, fp) == EOF) {
            ar__io__error("Failed to write memory count to %s", ctx->filename);
            ar__data__destroy(own_keys);
            _cleanup_agent_list(own_agents, own_items, total_agents);
            return false;
        }
        
        // Write each memory key-value pair
        while (ar__data__list_count(own_keys) > 0) {
            data_t *own_key_data = ar__data__list_remove_first(own_keys);
            if (!own_key_data) {
                break;
            }
            
            const char *key = ar__data__get_string(own_key_data);
            if (!key) {
                ar__data__destroy(own_key_data);
                continue;
            }
            
            // Get the value for this key
            data_t *ref_value = ar__data__get_map_data(ref_memory, key);
            if (!ref_value) {
                ar__data__destroy(own_key_data);
                continue;
            }
            
            // Write based on the value type
            data_type_t value_type = ar__data__get_type(ref_value);
            switch (value_type) {
                case DATA_INTEGER: {
                    // Write key and type
                    written = snprintf(buffer, sizeof(buffer), "%s int\n", key);
                    if (written < 0 || written >= (int)sizeof(buffer)) {
                        ar__io__error("Buffer too small for memory key/type in %s", ctx->filename);
                        ar__data__destroy(own_key_data);
                        ar__data__destroy(own_keys);
                        _cleanup_agent_list(own_agents, own_items, total_agents);
                        return false;
                    }
                    if (fputs(buffer, fp) == EOF) {
                        ar__io__error("Failed to write memory key/type to %s", ctx->filename);
                        ar__data__destroy(own_key_data);
                        ar__data__destroy(own_keys);
                        _cleanup_agent_list(own_agents, own_items, total_agents);
                        return false;
                    }
                    
                    // Write value
                    int value = ar__data__get_integer(ref_value);
                    written = snprintf(buffer, sizeof(buffer), "%d\n", value);
                    break;
                }
                case DATA_DOUBLE: {
                    // Write key and type
                    written = snprintf(buffer, sizeof(buffer), "%s double\n", key);
                    if (written < 0 || written >= (int)sizeof(buffer)) {
                        ar__io__error("Buffer too small for memory key/type in %s", ctx->filename);
                        ar__data__destroy(own_key_data);
                        ar__data__destroy(own_keys);
                        _cleanup_agent_list(own_agents, own_items, total_agents);
                        return false;
                    }
                    if (fputs(buffer, fp) == EOF) {
                        ar__io__error("Failed to write memory key/type to %s", ctx->filename);
                        ar__data__destroy(own_key_data);
                        ar__data__destroy(own_keys);
                        _cleanup_agent_list(own_agents, own_items, total_agents);
                        return false;
                    }
                    
                    // Write value
                    double value = ar__data__get_double(ref_value);
                    written = snprintf(buffer, sizeof(buffer), "%.6f\n", value);
                    break;
                }
                case DATA_STRING: {
                    // Write key and type
                    written = snprintf(buffer, sizeof(buffer), "%s string\n", key);
                    if (written < 0 || written >= (int)sizeof(buffer)) {
                        ar__io__error("Buffer too small for memory key/type in %s", ctx->filename);
                        ar__data__destroy(own_key_data);
                        ar__data__destroy(own_keys);
                        _cleanup_agent_list(own_agents, own_items, total_agents);
                        return false;
                    }
                    if (fputs(buffer, fp) == EOF) {
                        ar__io__error("Failed to write memory key/type to %s", ctx->filename);
                        ar__data__destroy(own_key_data);
                        ar__data__destroy(own_keys);
                        _cleanup_agent_list(own_agents, own_items, total_agents);
                        return false;
                    }
                    
                    // Write value
                    const char *value = ar__data__get_string(ref_value);
                    if (value) {
                        written = snprintf(buffer, sizeof(buffer), "%s\n", value);
                    } else {
                        written = -1;
                    }
                    break;
                }
                default:
                    // Skip unsupported types (LIST, MAP)
                    ar__data__destroy(own_key_data);
                    continue;
            }
            
            // Write the value line (written variable contains the result from switch statement)
            if (written < 0 || written >= (int)sizeof(buffer)) {
                ar__io__error("Buffer too small for memory value in %s", ctx->filename);
                ar__data__destroy(own_key_data);
                ar__data__destroy(own_keys);
                _cleanup_agent_list(own_agents, own_items, total_agents);
                return false;
            }
            
            if (fputs(buffer, fp) == EOF) {
                ar__io__error("Failed to write memory value to %s", ctx->filename);
                ar__data__destroy(own_key_data);
                ar__data__destroy(own_keys);
                _cleanup_agent_list(own_agents, own_items, total_agents);
                return false;
            }
            
            ar__data__destroy(own_key_data);
        }
        
        ar__data__destroy(own_keys);
    }
    
    // Clean up
    _cleanup_agent_list(own_agents, own_items, total_agents);
    
    return true;
}

/* Data structure to hold agent info during loading */
typedef struct {
    int64_t id;
    char method_name[256];
    char method_version[64];
} agent_load_info_t;

/* Validate the format of an agent store file */
static bool _validate_store_file(const char *filename, char *error_message, size_t error_size) {
    FILE *fp;
    file_result_t result = ar__io__open_file(filename, "r", &fp);
    
    if (result == FILE_ERROR_NOT_FOUND) {
        snprintf(error_message, error_size, "Agent store file %s not found", filename);
        return false;
    } else if (result != FILE_SUCCESS) {
        snprintf(error_message, error_size, "Failed to open agent store file: %s",
                ar__io__error_message(result));
        return false;
    }
    
    // Read and validate agent count
    char line[MAX_LINE_LENGTH] = {0};
    if (!ar__io__read_line(fp, line, (int)sizeof(line), filename)) {
        snprintf(error_message, error_size, "Failed to read agent count from %s", filename);
        ar__io__close_file(fp, filename);
        return false;
    }
    
    int count = 0;
    char *endptr = NULL;
    errno = 0;
    count = (int)strtol(line, &endptr, 10);
    
    if (errno != 0 || endptr == line || (*endptr != '\0' && *endptr != '\n') ||
        count < 0 || count > MAX_STORE_AGENTS) {
        snprintf(error_message, error_size, "Invalid agent count in %s", filename);
        ar__io__close_file(fp, filename);
        return false;
    }
    
    // Validate each agent entry
    for (int i = 0; i < count; i++) {
        // Read agent line
        if (!ar__io__read_line(fp, line, (int)sizeof(line), filename)) {
            snprintf(error_message, error_size, "Failed to read agent entry %d from %s", i+1, filename);
            ar__io__close_file(fp, filename);
            return false;
        }
        
        // Count tokens - should be exactly 3 (id, method_name, method_version)
        char line_copy[MAX_LINE_LENGTH];
        ar__io__string_copy(line_copy, line, sizeof(line_copy));
        
        char *saveptr = NULL;
        char *token = strtok_r(line_copy, " \t\n", &saveptr);
        int tokens = 0;
        while (token != NULL) {
            tokens++;
            token = strtok_r(NULL, " \t\n", &saveptr);
        }
        
        if (tokens != 3) {
            snprintf(error_message, error_size,
                    "Malformed agent entry %d in %s (expected 3 fields, got %d)",
                    i+1, filename, tokens);
            ar__io__close_file(fp, filename);
            return false;
        }
        
        // Read memory count
        if (!ar__io__read_line(fp, line, (int)sizeof(line), filename)) {
            snprintf(error_message, error_size,
                    "Failed to read memory count for agent %d in %s", i+1, filename);
            ar__io__close_file(fp, filename);
            return false;
        }
        
        // Parse memory count
        int mem_count = 0;
        endptr = NULL;
        errno = 0;
        mem_count = (int)strtol(line, &endptr, 10);
        
        if (errno != 0 || endptr == line || (*endptr != '\0' && *endptr != '\n') ||
            mem_count < 0 || mem_count > MAX_MEMORY_ITEMS) {
            snprintf(error_message, error_size,
                    "Invalid memory count for agent %d in %s", i+1, filename);
            ar__io__close_file(fp, filename);
            return false;
        }
        
        // Validate memory entries
        for (int j = 0; j < mem_count; j++) {
            // Read key/type line
            if (!ar__io__read_line(fp, line, (int)sizeof(line), filename)) {
                snprintf(error_message, error_size,
                        "Failed to read memory entry %d for agent %d in %s",
                        j+1, i+1, filename);
                ar__io__close_file(fp, filename);
                return false;
            }
            
            // Parse type to know if we need to read value line
            ar__io__string_copy(line_copy, line, sizeof(line_copy));
            saveptr = NULL;
            token = strtok_r(line_copy, " \t\n", &saveptr); // Skip key
            if (token == NULL) {
                snprintf(error_message, error_size,
                        "Malformed memory entry - missing key for agent %d in %s", i+1, filename);
                ar__io__close_file(fp, filename);
                return false;
            }
            
            token = strtok_r(NULL, " \t\n", &saveptr); // Get type
            if (token == NULL) {
                snprintf(error_message, error_size,
                        "Malformed memory entry - missing type for agent %d in %s", i+1, filename);
                ar__io__close_file(fp, filename);
                return false;
            }
            
            // Read value line for known types
            if (strcmp(token, "int") == 0 || strcmp(token, "double") == 0 ||
                strcmp(token, "string") == 0 || strcmp(token, "unknown") == 0) {
                if (!ar__io__read_line(fp, line, (int)sizeof(line), filename)) {
                    snprintf(error_message, error_size,
                            "Failed to read memory value for agent %d in %s", i+1, filename);
                    ar__io__close_file(fp, filename);
                    return false;
                }
            } else {
                snprintf(error_message, error_size,
                        "Unknown memory type '%s' for agent %d in %s", token, i+1, filename);
                ar__io__close_file(fp, filename);
                return false;
            }
        }
    }
    
    ar__io__close_file(fp, filename);
    return true;
}

/* Save all agents to persistent storage */
bool ar__agent_store__save(void) {
    // Set up context for writer function
    store_save_context_t context = {
        .filename = AGENT_STORE_FILE_NAME
    };
    
    // Create backup of existing file if it exists
    struct stat st;
    if (stat(AGENT_STORE_FILE_NAME, &st) == 0) {
        file_result_t backup_result = ar__io__create_backup(AGENT_STORE_FILE_NAME);
        if (backup_result != FILE_SUCCESS) {
            ar__io__warning("Failed to create backup of agent store file: %s",
                         ar__io__error_message(backup_result));
            // Continue despite backup failure
        } else {
            ar__io__info("Created backup of agent store file before saving");
        }
    }
    
    // Use the safe file writing utility
    file_result_t result = ar__io__write_file(AGENT_STORE_FILE_NAME, _store_write_function, &context);
    if (result != FILE_SUCCESS) {
        ar__io__error("Failed to save agent store file: %s", ar__io__error_message(result));
        
        // Try to restore backup if available
        if (stat(AGENT_STORE_FILE_NAME ".bak", &st) == 0) {
            ar__io__warning("Attempting to restore backup file after save failure");
            if (ar__io__restore_backup(AGENT_STORE_FILE_NAME) != FILE_SUCCESS) {
                ar__io__error("Failed to restore backup file");
            } else {
                ar__io__info("Successfully restored backup file after save failure");
            }
        }
        
        return false;
    }
    
    // Set secure permissions on the file
    result = ar__io__set_secure_permissions(AGENT_STORE_FILE_NAME);
    if (result != FILE_SUCCESS) {
        ar__io__warning("Failed to set secure permissions on agent store file: %s",
                     ar__io__error_message(result));
        // Continue despite permission issues
    }
    
    ar__io__info("Successfully saved %d agents to file", ar__agency__count_active_agents());
    return true;
}

/* Load all agents from persistent storage */
bool ar__agent_store__load(void) {
    // First validate the file format
    char error_message[512];
    if (!_validate_store_file(AGENT_STORE_FILE_NAME, error_message, sizeof(error_message))) {
        if (strstr(error_message, "not found") != NULL) {
            // Not an error, might be first run
            return true;
        }
        
        // File exists but has errors
        ar__io__error("Agent store file validation failed: %s", error_message);
        
        // Create a backup and remove the corrupted file
        ar__io__warning("Creating backup of corrupted agent store file");
        ar__io__create_backup(AGENT_STORE_FILE_NAME);
        remove(AGENT_STORE_FILE_NAME);
        
        return true; // Return success but with empty state
    }
    
    // Open the file (now that we know it's valid)
    FILE *fp;
    file_result_t result = ar__io__open_file(AGENT_STORE_FILE_NAME, "r", &fp);
    
    if (result != FILE_SUCCESS) {
        ar__io__error("Failed to open agent store file: %s", ar__io__error_message(result));
        return false;
    }
    
    // Read agent count
    char line[MAX_LINE_LENGTH] = {0};
    if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
        ar__io__error("Failed to read agent count from %s", AGENT_STORE_FILE_NAME);
        ar__io__close_file(fp, AGENT_STORE_FILE_NAME);
        return false;
    }
    
    // Parse agent count
    int count = 0;
    char *endptr = NULL;
    errno = 0;
    count = (int)strtol(line, &endptr, 10);
    
    if (count == 0) {
        ar__io__close_file(fp, AGENT_STORE_FILE_NAME);
        return true; // No agents to load
    }
    
    // Allocate array for agent info
    agent_load_info_t *own_agent_info = AR__HEAP__MALLOC(sizeof(agent_load_info_t) * (size_t)count, "agent info array");
    if (!own_agent_info) {
        ar__io__error("Failed to allocate memory for agent info");
        ar__io__close_file(fp, AGENT_STORE_FILE_NAME);
        return false;
    }
    
    // First pass: Read and validate all agent info
    bool validation_error = false;
    for (int i = 0; i < count; i++) {
        // Read agent line
        if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
            ar__io__error("Failed to read agent entry %d from %s", i+1, AGENT_STORE_FILE_NAME);
            validation_error = true;
            break;
        }
        
        // Parse agent info
        char *saveptr = NULL;
        char *token = strtok_r(line, " \t\n", &saveptr);
        if (token == NULL) {
            ar__io__error("Malformed agent entry %d - missing ID", i+1);
            validation_error = true;
            break;
        }
        
        // Parse ID
        errno = 0;
        own_agent_info[i].id = strtoll(token, &endptr, 10);
        if (errno != 0 || endptr == token) {
            ar__io__error("Invalid agent ID in entry %d", i+1);
            validation_error = true;
            break;
        }
        
        // Get method name
        token = strtok_r(NULL, " \t\n", &saveptr);
        if (token == NULL) {
            ar__io__error("Malformed agent entry %d - missing method name", i+1);
            validation_error = true;
            break;
        }
        ar__io__string_copy(own_agent_info[i].method_name, token, sizeof(own_agent_info[i].method_name));
        
        // Get method version
        token = strtok_r(NULL, " \t\n", &saveptr);
        if (token == NULL) {
            ar__io__error("Malformed agent entry %d - missing method version", i+1);
            validation_error = true;
            break;
        }
        ar__io__string_copy(own_agent_info[i].method_version, token, sizeof(own_agent_info[i].method_version));
        
        // Read memory count
        if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
            ar__io__error("Failed to read memory count for agent %" PRId64 "", own_agent_info[i].id);
            validation_error = true;
            break;
        }
        
        // Parse memory count
        int mem_count = 0;
        errno = 0;
        mem_count = (int)strtol(line, &endptr, 10);
        
        if (errno != 0 || endptr == line || mem_count < 0 || mem_count > MAX_MEMORY_ITEMS) {
            ar__io__error("Invalid memory count for agent %" PRId64 "", own_agent_info[i].id);
            validation_error = true;
            break;
        }
        
        // Skip memory entries during validation
        for (int j = 0; j < mem_count; j++) {
            // Read key/type line
            if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
                ar__io__error("Failed to read memory key/type for agent %" PRId64 "", own_agent_info[i].id);
                validation_error = true;
                break;
            }
            
            // Read value line
            if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
                ar__io__error("Failed to read memory value for agent %" PRId64 "", own_agent_info[i].id);
                validation_error = true;
                break;
            }
        }
        
        if (validation_error) {
            break;
        }
    }
    
    // If validation failed, clean up
    if (validation_error) {
        AR__HEAP__FREE(own_agent_info);
        ar__io__close_file(fp, AGENT_STORE_FILE_NAME);
        
        // File is corrupt, create backup and delete
        ar__io__warning("Agent store file appears to be corrupt. Creating backup and continuing with empty state.");
        ar__io__create_backup(AGENT_STORE_FILE_NAME);
        remove(AGENT_STORE_FILE_NAME);
        return true;
    }
    
    // Reset file position for second pass
    rewind(fp);
    
    // Skip the agent count line
    if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
        ar__io__error("Failed to skip agent count line");
        AR__HEAP__FREE(own_agent_info);
        ar__io__close_file(fp, AGENT_STORE_FILE_NAME);
        return false;
    }
    
    // Second pass: Create agents and load their state
    for (int i = 0; i < count; i++) {
        // Skip the agent info line (we already have it)
        if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
            ar__io__error("Failed to skip agent info line for agent %d", i+1);
            break;
        }
        
        // Create the agent
        int64_t new_id = ar__agency__create_agent(
            own_agent_info[i].method_name,
            own_agent_info[i].method_version,
            NULL);
        
        ar__io__info("Creating agent: method=%s, version=%s, new_id=%" PRId64 ", target_id=%" PRId64 "",
                   own_agent_info[i].method_name,
                   own_agent_info[i].method_version,
                   new_id,
                   own_agent_info[i].id);
        
        if (new_id == 0) {
            ar__io__error("Could not recreate agent %" PRId64 "", own_agent_info[i].id);
            
            // Skip memory data
            if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
                ar__io__error("Failed to skip memory count for failed agent creation");
                break;
            }
            
            int mem_count = 0;
            errno = 0;
            mem_count = (int)strtol(line, &endptr, 10);
            if (errno == 0 && mem_count > 0) {
                for (int k = 0; k < mem_count; k++) {
                    if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) break;
                    if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) break;
                }
            }
            
            continue;
        }
        
        // Update the assigned ID to match the stored one
        bool id_updated = ar__agency__set_agent_id(new_id, own_agent_info[i].id);
        ar__io__info("ID update result: %s (from %" PRId64 " to %" PRId64 ")", 
                   id_updated ? "success" : "failure", 
                   new_id, own_agent_info[i].id);
        if (!id_updated) {
            ar__io__error("Failed to update agent ID from %" PRId64 " to %" PRId64 "", new_id, own_agent_info[i].id);
        }
        
        // Read memory count
        if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
            ar__io__error("Failed to read memory count for agent %" PRId64 "", own_agent_info[i].id);
            continue;
        }
        
        // Parse memory count
        int mem_count = 0;
        errno = 0;
        mem_count = (int)strtol(line, &endptr, 10);
        
        if (errno != 0 || endptr == line || mem_count < 0 || mem_count > MAX_MEMORY_ITEMS) {
            ar__io__error("Invalid memory count for agent %" PRId64 "", own_agent_info[i].id);
            continue;
        }
        
        // Get mutable memory for this agent
        data_t *mut_memory = ar__agency__get_agent_mutable_memory(own_agent_info[i].id);
        if (!mut_memory && mem_count > 0) {
            ar__io__error("Agent %" PRId64 " has no memory map but file indicates %d items", 
                       own_agent_info[i].id, mem_count);
            // Skip memory items
            for (int k = 0; k < mem_count; k++) {
                if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) break;
                if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) break;
            }
            continue;
        }
        
        // Process each memory item
        for (int k = 0; k < mem_count; k++) {
            char key[256] = {0};
            char type[32] = {0};
            
            // Read key and type line
            if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
                ar__io__error("Failed to read memory key/type for agent %" PRId64 "", own_agent_info[i].id);
                break;
            }
            
            // Parse key and type
            char *saveptr = NULL;
            char *token = strtok_r(line, " \t\n", &saveptr);
            if (token == NULL) {
                ar__io__error("Malformed memory entry - missing key for agent %" PRId64 "", own_agent_info[i].id);
                break;
            }
            ar__io__string_copy(key, token, sizeof(key));
            
            token = strtok_r(NULL, " \t\n", &saveptr);
            if (token == NULL) {
                ar__io__error("Malformed memory entry - missing type for agent %" PRId64 "", own_agent_info[i].id);
                break;
            }
            ar__io__string_copy(type, token, sizeof(type));
            
            // Process based on type
            data_t *own_value = NULL;
            
            if (strcmp(type, "int") == 0) {
                // Read value line
                if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
                    ar__io__error("Failed to read int value for agent %" PRId64 "", own_agent_info[i].id);
                    break;
                }
                
                // Parse int value
                int int_value;
                errno = 0;
                int_value = (int)strtol(line, &endptr, 10);
                
                if (errno != 0 || endptr == line) {
                    ar__io__error("Invalid int value for agent %" PRId64 "", own_agent_info[i].id);
                    break;
                }
                
                own_value = ar__data__create_integer(int_value);
            } else if (strcmp(type, "double") == 0) {
                // Read value line
                if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
                    ar__io__error("Failed to read double value for agent %" PRId64 "", own_agent_info[i].id);
                    break;
                }
                
                // Parse double value
                double double_value;
                errno = 0;
                double_value = strtod(line, &endptr);
                
                if (errno != 0 || endptr == line) {
                    ar__io__error("Invalid double value for agent %" PRId64 "", own_agent_info[i].id);
                    break;
                }
                
                own_value = ar__data__create_double(double_value);
            } else if (strcmp(type, "string") == 0) {
                // Read value line
                if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
                    ar__io__error("Failed to read string value for agent %" PRId64 "", own_agent_info[i].id);
                    break;
                }
                
                // Remove trailing newline if present
                size_t len = strlen(line);
                if (len > 0 && line[len-1] == '\n') {
                    line[len-1] = '\0';
                }
                
                own_value = ar__data__create_string(line);
            } else {
                // Skip unknown type
                ar__io__warning("Unknown memory type '%s' for agent %" PRId64 "", type, own_agent_info[i].id);
                if (!ar__io__read_line(fp, line, (int)sizeof(line), AGENT_STORE_FILE_NAME)) {
                    ar__io__error("Could not skip unknown type for agent %" PRId64 "", own_agent_info[i].id);
                    break;
                }
                continue;
            }
            
            // Add the value to the agent's memory map
            if (own_value && mut_memory) {
                ar__data__set_map_data(mut_memory, key, own_value);
                // Ownership transferred
                own_value = NULL;
            } else if (own_value) {
                ar__io__error("Failed to set value for agent %" PRId64 "", own_agent_info[i].id);
                ar__data__destroy(own_value);
            }
        }
        
        // Update next_agent_id if needed to prevent ID collision
        agent_registry_t *ref_registry = ar__agency__get_registry();
        if (ref_registry) {
            int64_t next_id = ar__agent_registry__get_next_id(ref_registry);
            if (own_agent_info[i].id >= next_id) {
                ar__agent_registry__set_next_id(ref_registry, own_agent_info[i].id + 1);
            }
        }
    }
    
    // Clean up
    AR__HEAP__FREE(own_agent_info);
    ar__io__close_file(fp, AGENT_STORE_FILE_NAME);
    
    return true;
}

/* Check if agent store file exists */
bool ar__agent_store__exists(void) {
    struct stat st;
    return (stat(AGENT_STORE_FILE_NAME, &st) == 0);
}

/* Delete the agent store file */
bool ar__agent_store__delete(void) {
    if (!ar__agent_store__exists()) {
        return true; // Already doesn't exist
    }
    
    // Create backup before deletion
    file_result_t backup_result = ar__io__create_backup(AGENT_STORE_FILE_NAME);
    if (backup_result != FILE_SUCCESS) {
        ar__io__warning("Failed to create backup before deletion: %s",
                     ar__io__error_message(backup_result));
    }
    
    if (remove(AGENT_STORE_FILE_NAME) != 0) {
        ar__io__error("Failed to delete agent store file: %s", strerror(errno));
        return false;
    }
    
    ar__io__info("Agent store file deleted (backup created)");
    return true;
}

/* Get the path to the agent store file */
const char* ar__agent_store__get_path(void) {
    return AGENT_STORE_FILE_NAME;
}
