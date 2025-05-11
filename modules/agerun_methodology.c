#include "agerun_methodology.h"
#include "agerun_method.h"
#include "agerun_string.h"
#include "agerun_heap.h"
#include "agerun_semver.h"
#include "agerun_agency.h"
#include "agerun_io.h" /* Include the I/O utilities */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

/* Constants */
#define MAX_METHODS 100
#define MAX_METHOD_NAME_LENGTH 256
#define MAX_VERSIONS_PER_METHOD 32

/**
 * Validates the format of a methodology file to ensure it can be loaded
 * Provides detailed error message in case of validation failure
 *
 * @param filename Path to the methodology file
 * @param error_message Buffer to store error message if validation fails
 * @param error_size Size of the error message buffer
 * @return true if file is valid, false if there are formatting issues
 */
static bool ar_methodology_validate_file(const char *filename, char *error_message, size_t error_size) {
    FILE *fp;
    file_result_t result = ar_io_open_file(filename, "r", &fp);

    if (result == FILE_ERROR_NOT_FOUND) {
        snprintf(error_message, error_size, "Methodology file %s not found", filename);
        return false;
    } else if (result != FILE_SUCCESS) {
        snprintf(error_message, error_size, "Failed to open methodology file: %s",
                ar_io_error_message(result));
        return false;
    }

    // Read and validate method count
    char line[256] = {0};
    if (!ar_io_read_line(fp, line, (int)sizeof(line), filename)) {
        snprintf(error_message, error_size, "Failed to read method count from %s", filename);
        ar_io_close_file(fp, filename);
        return false;
    }

    int method_count = 0;
    char *line_endptr = NULL;
    errno = 0;
    method_count = (int)strtol(line, &line_endptr, 10);

    if (errno != 0 || line_endptr == line || (*line_endptr != '\0' && *line_endptr != '\n') ||
        method_count < 0 || method_count > MAX_METHODS) {
        snprintf(error_message, error_size, "Invalid method count in %s", filename);
        ar_io_close_file(fp, filename);
        return false;
    }

    // Validate each method entry
    for (int i = 0; i < method_count; i++) {
        // Read method name and version count line
        if (!ar_io_read_line(fp, line, (int)sizeof(line), filename)) {
            snprintf(error_message, error_size,
                    "Failed to read method entry %d from %s", i+1, filename);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Count tokens - should be exactly 2 (method_name, version_count)
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

        if (tokens != 2) {
            snprintf(error_message, error_size,
                    "Malformed method entry for method %d in %s: expected 2 fields, found %d",
                    i+1, filename, tokens);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Validate method name and version count format
        char method_name[MAX_METHOD_NAME_LENGTH] = {0};
        token = strtok_r(line, " \t\n", &next_token);
        if (token == NULL) {
            snprintf(error_message, error_size,
                    "Malformed method entry - missing method name in %s for method %d",
                    filename, i+1);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Copy method name with secure length check
        size_t token_len = strlen(token);
        if (token_len >= MAX_METHOD_NAME_LENGTH) {
            snprintf(error_message, error_size,
                    "Method name too long in %s for method %d", filename, i+1);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Use memcpy with explicit null termination for safer copy
        memcpy(method_name, token, token_len);
        method_name[token_len] = '\0';  // Ensure null-termination

        // Validate method name is not empty
        if (strlen(method_name) == 0) {
            snprintf(error_message, error_size,
                    "Empty method name in %s for method %d", filename, i+1);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Validate version count exists
        token = strtok_r(NULL, " \t\n", &next_token);
        if (token == NULL) {
            snprintf(error_message, error_size,
                    "Malformed method entry - missing version count in %s for method %d",
                    filename, i+1);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Validate version count is a number
        int version_count = 0;
        char *version_count_endptr = NULL;
        errno = 0;
        version_count = (int)strtol(token, &version_count_endptr, 10);
        if (errno != 0 || version_count_endptr == token || *version_count_endptr != '\0' ||
            version_count <= 0 || version_count > MAX_VERSIONS_PER_METHOD) {
            snprintf(error_message, error_size,
                    "Invalid version count '%s' for method %s in %s",
                    token, method_name, filename);
            ar_io_close_file(fp, filename);
            return false;
        }

        // Validate each version for this method
        for (int j = 0; j < version_count; j++) {
            // Read version line
            if (!ar_io_read_line(fp, line, (int)sizeof(line), filename)) {
                snprintf(error_message, error_size,
                        "Failed to read version for method %s (%d/%d) in %s",
                        method_name, j+1, version_count, filename);
                ar_io_close_file(fp, filename);
                return false;
            }

            // Validate version string
            if (strlen(line) == 0 || (line[0] == '\n' && strlen(line) == 1)) {
                snprintf(error_message, error_size,
                        "Empty version string for method %s (%d/%d) in %s",
                        method_name, j+1, version_count, filename);
                ar_io_close_file(fp, filename);
                return false;
            }

            // Read instructions line
            if (!ar_io_read_line(fp, line, (int)sizeof(line), filename)) {
                snprintf(error_message, error_size,
                        "Failed to read instructions for method %s (%d/%d) in %s",
                        method_name, j+1, version_count, filename);
                ar_io_close_file(fp, filename);
                return false;
            }

            // Validate instructions are not empty
            if (strlen(line) == 0 || (line[0] == '\n' && strlen(line) == 1)) {
                snprintf(error_message, error_size,
                        "Empty instructions for method %s (%d/%d) in %s",
                        method_name, j+1, version_count, filename);
                ar_io_close_file(fp, filename);
                return false;
            }
        }
    }

    ar_io_close_file(fp, filename);
    return true;
}

/* Constants */
#define MAX_INSTRUCTIONS_LENGTH 16384

/* Global State */
static method_t* methods[MAX_METHODS][MAX_VERSIONS_PER_METHOD];
static int method_counts[MAX_METHODS];
static int method_name_count = 0;

/**
 * Creates a new method object and registers it with the methodology module
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return true if method was created and registered successfully, false otherwise
 * @note Ownership: This function creates and takes ownership of the method.
 *       The caller should not worry about destroying the method.
 */
bool ar_methodology_create_method(const char *ref_name, const char *ref_instructions, 
                              const char *ref_version) {
    if (!ref_name || !ref_instructions) {
        return false;
    }
    
    // Create the method directly with the provided parameters
    method_t *own_method = ar_method_create(ref_name, ref_instructions, ref_version);
    if (!own_method) {
        return false;
    }
    
    // Register the method (transfers ownership)
    ar_methodology_register_method(own_method);
    own_method = NULL; // Mark as transferred
    
    return true;
}

/* Forward Declarations */
static method_t* find_latest_method(const char *ref_name);
static method_t* find_method(const char *ref_name, const char *ref_version);

/* Method Search Functions */
int ar_methodology_find_method_idx(const char *ref_name) {
    for (int i = 0; i < method_name_count; i++) {
        if (methods[i][0] != NULL && strcmp(ar_method_get_name(methods[i][0]), ref_name) == 0) {
            return i;
        }
    }
    
    return -1;
}

static method_t* find_latest_method(const char *ref_name) {
    int method_idx = ar_methodology_find_method_idx(ref_name);
    if (method_idx < 0 || method_counts[method_idx] == 0) {
        return NULL;
    }
    
    // Use semantic versioning to find the latest version
    int latest_idx = 0;
    for (int i = 1; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i] != NULL && methods[method_idx][latest_idx] != NULL &&
            ar_semver_compare(
                ar_method_get_version(methods[method_idx][i]),
                ar_method_get_version(methods[method_idx][latest_idx])
            ) > 0) {
            latest_idx = i;
        }
    }
    
    if (latest_idx >= 0 && methods[method_idx][latest_idx] != NULL) {
        return methods[method_idx][latest_idx];
    }
    
    return NULL;
}

static method_t* find_method(const char *ref_name, const char *ref_version) {
    int method_idx = ar_methodology_find_method_idx(ref_name);
    if (method_idx < 0) {
        return NULL;
    }
    
    // Case 1: If version is NULL, return the latest method
    if (ref_version == NULL) {
        return find_latest_method(ref_name);
    }
    
    // Case 2: Exact version match
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i] != NULL && 
            strcmp(ar_method_get_version(methods[method_idx][i]), ref_version) == 0) {
            return methods[method_idx][i];
        }
    }
    
    // Case 3: Partial version match (e.g., "1" or "1.2")
    // Collect all versions for this method
    const char *versions[MAX_VERSIONS_PER_METHOD];
    int valid_count = 0;
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i] != NULL) {
            versions[valid_count++] = ar_method_get_version(methods[method_idx][i]);
        }
    }
    
    // Find the latest version matching the pattern
    int latest_idx = ar_semver_find_latest_matching(versions, valid_count, ref_version);
    if (latest_idx >= 0) {
        // Map back to the original index in methods array
        for (int i = 0; i < method_counts[method_idx]; i++) {
            if (methods[method_idx][i] != NULL && 
                strcmp(ar_method_get_version(methods[method_idx][i]), versions[latest_idx]) == 0) {
                return methods[method_idx][i];
            }
        }
    }
    
    return NULL; // No matching version found
}

// This function is now implemented directly above

method_t* ar_methodology_get_method_storage(int method_idx, int version_idx) {
    AR_ASSERT(method_idx >= 0 && method_idx < MAX_METHODS, "Method index out of bounds");
    AR_ASSERT(version_idx >= 0 && version_idx < MAX_VERSIONS_PER_METHOD, "Version index out of bounds");
    return methods[method_idx][version_idx];
}

void ar_methodology_set_method_storage(int method_idx, int version_idx, method_t *ref_method) {
    AR_ASSERT(method_idx >= 0 && method_idx < MAX_METHODS, "Method index out of bounds");
    AR_ASSERT(version_idx >= 0 && version_idx < MAX_VERSIONS_PER_METHOD, "Version index out of bounds");
    
    // If there's already a method at this location, destroy it first
    if (methods[method_idx][version_idx] != NULL) {
        ar_method_destroy(methods[method_idx][version_idx]);
    }
    
    // Store the new method
    methods[method_idx][version_idx] = ref_method;
}

int* ar_methodology_get_method_counts(void) {
    return method_counts;
}

int* ar_methodology_get_method_name_count(void) {
    return &method_name_count;
}

// Main method access function
method_t* ar_methodology_get_method(const char *ref_name, const char *ref_version) {
    if (ref_version == NULL) {
        // Use latest version
        return find_latest_method(ref_name);
    } else {
        // Use specific version
        return find_method(ref_name, ref_version);
    }
}

bool ar_methodology_save_methods(void) {
    // Create a temporary file first, then rename it to avoid data corruption
    const char *temp_filename = METHODOLOGY_FILE_NAME ".tmp";
    
    // Use restricted mode "w" to ensure we only create/write, not execute
    FILE *mut_fp;
    file_result_t result = ar_io_open_file(temp_filename, "w", &mut_fp);
    if (result != FILE_SUCCESS) {
        ar_io_error("Could not open %s for writing: %s",
                temp_filename, ar_io_error_message(result));
        return false;
    }
    
    // Set secure file permissions for the file (restrict to owner read/write)
    #ifdef _WIN32
    _chmod(temp_filename, _S_IREAD | _S_IWRITE);
    #else
    chmod(temp_filename, S_IRUSR | S_IWUSR);
    #endif
    
    // Define buffer size constant for clarity and maintainability
    const size_t BUFFER_SIZE = 16384; // Increased buffer size for larger instructions
    char buffer[BUFFER_SIZE]; // Buffer for formatted output
    
    // Write method count
    int written = snprintf(buffer, BUFFER_SIZE, "%d\n", method_name_count);
    if (written < 0 || written >= (int)BUFFER_SIZE) {
        ar_io_error("Buffer too small for method count in %s", temp_filename);
        ar_io_close_file(mut_fp, temp_filename);
        remove(temp_filename);
        return false;
    }
    
    // Write the count to the file with error checking
    if (fputs(buffer, mut_fp) == EOF) {
        ar_io_error("Failed to write method count to %s", temp_filename);
        ar_io_close_file(mut_fp, temp_filename);
        remove(temp_filename);
        return false;
    }
    
    // For each method type
    for (int i = 0; i < method_name_count; i++) {
        if (methods[i][0] == NULL) {
            continue; // Skip empty method entries
        }
        
        // Get method name with validation
        const char *method_name = ar_method_get_name(methods[i][0]);
        if (!method_name) {
            ar_io_error("NULL method name at index %d", i);
            ar_io_close_file(mut_fp, temp_filename);
            remove(temp_filename);
            return false;
        }
        
        // Write method name and version count
        written = snprintf(buffer, BUFFER_SIZE, "%s %d\n", method_name, method_counts[i]);
        if (written < 0 || written >= (int)BUFFER_SIZE) {
            ar_io_error("Buffer too small for method data in %s", temp_filename);
            ar_io_close_file(mut_fp, temp_filename);
            remove(temp_filename);
            return false;
        }
        
        // Write the data to the file with error checking
        if (fputs(buffer, mut_fp) == EOF) {
            ar_io_error("Failed to write method data to %s", temp_filename);
            ar_io_close_file(mut_fp, temp_filename);
            remove(temp_filename);
            return false;
        }
        
        // For each version of this method
        for (int j = 0; j < method_counts[i]; j++) {
            if (methods[i][j] == NULL) {
                continue; // Skip NULL entries
            }
            
            const method_t *ref_method = methods[i][j];
            
            // Get version string with validation
            const char *version = ar_method_get_version(ref_method);
            if (!version) {
                ar_io_error("NULL version for method %s at index %d", method_name, j);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                remove(METHODOLOGY_FILE_NAME);
                return false;
            }
            
            // Write version string
            written = snprintf(buffer, BUFFER_SIZE, "%s\n", version);
            if (written < 0 || written >= (int)BUFFER_SIZE) {
                ar_io_error("Buffer too small for version data in %s", temp_filename);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                remove(METHODOLOGY_FILE_NAME);
                return false;
            }
            
            // Write the version to the file with error checking
            if (fputs(buffer, mut_fp) == EOF) {
                ar_io_error("Failed to write version data to %s", temp_filename);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                remove(METHODOLOGY_FILE_NAME);
                return false;
            }
            
            // Get instructions with validation
            const char *instructions = ar_method_get_instructions(ref_method);
            if (!instructions) {
                ar_io_error("NULL instructions for method %s version %s",
                        method_name, version);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                remove(METHODOLOGY_FILE_NAME);
                return false;
            }
            
            // Check if instructions will fit in buffer
            size_t instr_len = strlen(instructions);
            if (instr_len >= BUFFER_SIZE - 2) { // -2 for newline and null terminator
                ar_io_error("Instructions too large for buffer (size: %zu) for method %s version %s",
                        instr_len, method_name, version);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                remove(METHODOLOGY_FILE_NAME);
                return false;
            }
            
            // Write instructions - these can be large, so buffer size is critical
            written = snprintf(buffer, BUFFER_SIZE, "%s\n", instructions);
            if (written < 0 || written >= (int)BUFFER_SIZE) {
                ar_io_error("Buffer overflow writing instructions for method %s version %s",
                        method_name, version);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                remove(METHODOLOGY_FILE_NAME);
                return false;
            }
            
            // Write the instructions to the file with error checking
            if (fputs(buffer, mut_fp) == EOF) {
                ar_io_error("Failed to write instructions to %s", temp_filename);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                remove(METHODOLOGY_FILE_NAME);
                return false;
            }
        }
    }
    
    // Flush and close the file
    if (fflush(mut_fp) != 0) {
        ar_io_error("Failed to flush data to %s", temp_filename);
        ar_io_close_file(mut_fp, temp_filename);
        remove(temp_filename);
        return false;
    }

    ar_io_close_file(mut_fp, temp_filename);
    
    // Rename the temporary file to the permanent file
    if (rename(temp_filename, METHODOLOGY_FILE_NAME) != 0) {
        ar_io_error("Failed to rename %s to %s", temp_filename, METHODOLOGY_FILE_NAME);
        remove(temp_filename);
        return false;
    }
    
    return true;
}

void ar_methodology_cleanup(void) {
    // Free all method pointers in the methods array
    for (int i = 0; i < MAX_METHODS; i++) {
        for (int j = 0; j < MAX_VERSIONS_PER_METHOD; j++) {
            if (methods[i][j] != NULL) {
                ar_method_destroy(methods[i][j]);
                methods[i][j] = NULL;
            }
        }
        method_counts[i] = 0;
    }
    method_name_count = 0;
}

void ar_methodology_register_method(method_t *own_method) {
    if (!own_method) {
        return;
    }
    
    const char *method_name = ar_method_get_name(own_method);
    const char *method_version = ar_method_get_version(own_method);
    
    // Find or create a method index for this name
    int method_idx = ar_methodology_find_method_idx(method_name);
    if (method_idx < 0) {
        // No existing method with this name, create a new entry
        if (method_name_count >= MAX_METHODS) {
            ar_io_error("Maximum number of method types reached");
            ar_method_destroy(own_method); // Clean up the method
            return;
        }
        
        method_idx = method_name_count++;
    }
    
    // Check if we've reached max versions for this method
    if (method_counts[method_idx] >= MAX_VERSIONS_PER_METHOD) {
        ar_io_error("Maximum number of versions reached for method %s", method_name);
        ar_method_destroy(own_method); // Clean up the method
        return;
    }
    
    // Check for version conflicts
    bool version_conflict = false;
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i] != NULL && 
            strcmp(ar_method_get_version(methods[method_idx][i]), method_version) == 0) {
            version_conflict = true;
            break;
        }
    }
    
    if (version_conflict) {
        // Rather than modifying the version, we'll just append a note
        ar_io_warning("Method %s version %s already exists", method_name, method_version);
    }
    
    // Find the next version slot
    int version_idx = method_counts[method_idx];
    
    // Store the method in our methods array, handling any existing method
    ar_methodology_set_method_storage(method_idx, version_idx, own_method);
    method_counts[method_idx]++;
    
    ar_io_info("Registered method %s version %s", method_name, method_version);
    
    // Update agents using compatible older versions to use this new version
    // We only auto-update if the new version is higher than existing versions
    for (int i = 0; i < method_counts[method_idx] - 1; i++) {
        if (methods[method_idx][i] != NULL) {
            // Check if the old method is compatible with the new one
            if (ar_semver_are_compatible(
                    ar_method_get_version(methods[method_idx][i]), 
                    method_version)) {
                
                // Check if the new version is higher
                if (ar_semver_compare(
                        method_version,
                        ar_method_get_version(methods[method_idx][i])) > 0) {
                    
                    // Update agents using the old method to use the new one
                    int updated = ar_agency_update_agent_methods(
                        methods[method_idx][i], 
                        own_method);
                    
                    if (updated > 0) {
                        ar_io_info("Updated %d agent(s) from method %s version %s to version %s",
                               updated, method_name,
                               ar_method_get_version(methods[method_idx][i]),
                               method_version);
                    }
                }
            }
        }
    }
}

bool ar_methodology_load_methods(void) {
    // First validate the methodology file format
    char error_message[512];
    if (!ar_methodology_validate_file(METHODOLOGY_FILE_NAME, error_message, sizeof(error_message))) {
        if (strstr(error_message, "not found") != NULL) {
            // Not an error, might be first run
            return true;
        }

        // File exists but has errors
        ar_io_error("Methodology file validation failed: %s", error_message);

        // Create a backup and remove the corrupted file
        ar_io_warning("Creating backup of corrupted methodology file");
        ar_io_create_backup(METHODOLOGY_FILE_NAME);
        remove(METHODOLOGY_FILE_NAME);

        return true; // Return success but with empty state
    }

    // Securely open the file with proper error handling (now that we know it's valid)
    FILE *mut_fp;
    file_result_t result = ar_io_open_file(METHODOLOGY_FILE_NAME, "r", &mut_fp);

    if (result != FILE_SUCCESS) {
        ar_io_error("Failed to open methodology file: %s", ar_io_error_message(result));
        return false;
    }

    // Verify file permissions (should be readable only by owner)
    result = ar_io_set_secure_permissions(METHODOLOGY_FILE_NAME);
    if (result != FILE_SUCCESS) {
        ar_io_warning("Failed to set secure permissions on methodology file: %s",
                   ar_io_error_message(result));
        // Continue anyway with a warning
    }
    
    // Define buffer size and initialize line buffer securely
    const size_t LINE_SIZE = 256;
    char line[LINE_SIZE] = {0}; // Initialize to zeros for security
    
    // Read the first line to get method count with error checking
    if (!ar_io_read_line(mut_fp, line, (int)LINE_SIZE, METHODOLOGY_FILE_NAME)) {
        ar_io_error("Failed to read method count from %s", METHODOLOGY_FILE_NAME);
        ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);

        // Create a backup and remove the corrupted file
        ar_io_warning("Creating backup of corrupted methodology file");
        ar_io_create_backup(METHODOLOGY_FILE_NAME);
        remove(METHODOLOGY_FILE_NAME);

        return true; // Return success with empty state
    }
    
    // Parse the method count using strtol with error checking
    int method_count = 0;
    char *method_count_endptr = NULL;
    errno = 0; // Reset errno to check for conversion errors
    method_count = (int)strtol(line, &method_count_endptr, 10);
    
    // Validate conversion success and range
    if (errno != 0 || method_count_endptr == line ||
        (*method_count_endptr != '\0' && *method_count_endptr != '\n') ||
        method_count <= 0 || method_count > MAX_METHODS) {
        ar_io_error("Invalid method count in %s", METHODOLOGY_FILE_NAME);
        ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);

        // Create a backup and delete the corrupted file
        ar_io_warning("Creating backup of corrupted methodology file");
        ar_io_create_backup(METHODOLOGY_FILE_NAME);
        remove(METHODOLOGY_FILE_NAME);
        return true;
    }
    
    // Clear existing methods to avoid conflicts using ar_method_destroy
    for (int i = 0; i < MAX_METHODS; i++) {
        for (int j = 0; j < MAX_VERSIONS_PER_METHOD; j++) {
            if (methods[i][j] != NULL) {
                ar_method_destroy(methods[i][j]);
                methods[i][j] = NULL;
            }
        }
        method_counts[i] = 0;
    }
    method_name_count = 0;
    
    // Process each method type
    for (int i = 0; i < method_count; i++) {
        char name[MAX_METHOD_NAME_LENGTH] = {0}; // Initialize to zeros for security
        int version_count = 0;
        
        // Read the next line for method name and version count
        if (fgets(line, LINE_SIZE, mut_fp) == NULL) {
            ar_io_error("Unexpected end of file in %s (method %d)",
                    METHODOLOGY_FILE_NAME, i+1);
            ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
            // Delete the corrupted file and start fresh
            ar_io_warning("Deleting corrupted methodology file");
            remove(METHODOLOGY_FILE_NAME);
            return true;
        }
        
        // Parse the method name and version count manually using strtok_r
        char *token = NULL;
        char *next_token = NULL;
        
        // Get method name
        token = strtok_r(line, " \t\n", &next_token);
        if (token == NULL) {
            ar_io_error("Malformed method entry - missing name in %s (method %d)",
                    METHODOLOGY_FILE_NAME, i+1);
            ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
            ar_io_warning("Deleting corrupted methodology file");
            remove(METHODOLOGY_FILE_NAME);
            return true;
        }
        
        // Copy method name with secure length check
        size_t token_len = strlen(token);
        if (token_len >= MAX_METHOD_NAME_LENGTH) {
            ar_io_error("Method name too long in %s (method %d)",
                    METHODOLOGY_FILE_NAME, i+1);
            ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
            ar_io_warning("Deleting corrupted methodology file");
            remove(METHODOLOGY_FILE_NAME);
            return true;
        }

        // Use memcpy for secure copying with guaranteed null termination
        // token_len is already computed above

        memcpy(name, token, token_len);
        name[token_len] = '\0';  // Ensure null-termination
        
        // Get version count
        token = strtok_r(NULL, " \t\n", &next_token);
        if (token == NULL) {
            ar_io_error("Malformed method entry - missing version count in %s (method %d)",
                    METHODOLOGY_FILE_NAME, i+1);
            ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
            ar_io_warning("Deleting corrupted methodology file");
            remove(METHODOLOGY_FILE_NAME);
            return true;
        }
        
        // Convert version count to int with error checking
        char *version_count_endptr = NULL;
        errno = 0; // Reset errno to check for conversion errors
        version_count = (int)strtol(token, &version_count_endptr, 10);
        
        // Validate conversion success and range
        if (errno != 0 || version_count_endptr == token || *version_count_endptr != '\0' ||
            version_count <= 0 || version_count > MAX_VERSIONS_PER_METHOD) {
            ar_io_error("Invalid version count for method %s in %s",
                    name, METHODOLOGY_FILE_NAME);
            ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
            ar_io_warning("Deleting corrupted methodology file");
            remove(METHODOLOGY_FILE_NAME);
            return true;
        }
        
        // Add this method to the methods array
        int method_idx = method_name_count++;
        method_counts[method_idx] = 0; // Reset count for this method
        
        // For each version of this method
        for (int j = 0; j < version_count; j++) {
            // Verify we don't exceed the maximum versions per method
            if (method_counts[method_idx] >= MAX_VERSIONS_PER_METHOD) {
                ar_io_error("Maximum versions reached for method %s in %s",
                        name, METHODOLOGY_FILE_NAME);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                return false;
            }
            
            // Buffer for version string, initialized to zeros
            char version[64] = {0}; 
            
            // Read version string with error checking
            if (fgets(line, LINE_SIZE, mut_fp) == NULL) {
                ar_io_error("Unexpected end of file in %s when reading version for method %s",
                        METHODOLOGY_FILE_NAME, name);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                ar_io_warning("Deleting corrupted methodology file");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Remove trailing newline if present
            size_t len = strlen(line);
            if (len > 0 && line[len-1] == '\n') {
                line[len-1] = '\0';
            }
            
            // Validate version string length
            if (len == 0) {
                ar_io_error("Empty version string for method %s in %s",
                        name, METHODOLOGY_FILE_NAME);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                ar_io_warning("Deleting corrupted methodology file");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Copy the version string securely
            if (len > sizeof(version) - 1) {
                ar_io_error("Version string too long for method %s in %s",
                        name, METHODOLOGY_FILE_NAME);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                ar_io_warning("Deleting corrupted methodology file");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            strncpy(version, line, sizeof(version) - 1);
            version[sizeof(version) - 1] = '\0';  // Ensure null-termination
            
            // Buffer for instructions with security initialization
            char instructions[MAX_INSTRUCTIONS_LENGTH] = {0};
            
            // Read the instructions with error checking
            if (fgets(instructions, MAX_INSTRUCTIONS_LENGTH, mut_fp) == NULL) {
                ar_io_error("Could not read instructions for method %s version %s in %s",
                       name, version, METHODOLOGY_FILE_NAME);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                ar_io_warning("Deleting corrupted methodology file");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Validate instructions
            size_t instr_len = strlen(instructions);
            if (instr_len == 0) {
                ar_io_error("Empty instructions for method %s version %s in %s",
                        name, version, METHODOLOGY_FILE_NAME);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                ar_io_warning("Deleting corrupted methodology file");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Remove trailing newline if present
            if (instructions[instr_len-1] == '\n') {
                instructions[instr_len-1] = '\0';
            }
            
            // Check for truncation (buffer filled completely)
            if (instr_len >= MAX_INSTRUCTIONS_LENGTH - 1) {
                ar_io_warning("Instructions for method %s version %s may be truncated in %s",
                        name, version, METHODOLOGY_FILE_NAME);
            }
            
            // Create a new method with data from the file
            method_t *own_method = ar_method_create(name, instructions, version);
            
            if (own_method) {
                // Store the method in the methods array
                ar_methodology_set_method_storage(method_idx, method_counts[method_idx]++, own_method);
                own_method = NULL; // Mark as transferred
            } else {
                ar_io_error("Failed to create method %s version %s", name, version);
                ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
                return false;
            }
        }
    }
    
    ar_io_close_file(mut_fp, METHODOLOGY_FILE_NAME);
    return true;
}
