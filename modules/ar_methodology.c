#include "ar_methodology.h"
#include "ar_method.h"
#include "ar_string.h"
#include "ar_heap.h"
#include "ar_semver.h"
#include "ar_agency.h"
#include "ar_io.h" /* Include the I/O utilities */
#include "ar_assert.h" /* Include the assertion utilities */

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
#define BUFFER_SIZE 16384  /* Buffer size for save operations */
#define LINE_SIZE 256      /* Line buffer size for load operations */

/**
 * Validates the format of a methodology file to ensure it can be loaded
 * Provides detailed error message in case of validation failure
 *
 * @param filename Path to the methodology file
 * @param error_message Buffer to store error message if validation fails
 * @param error_size Size of the error message buffer
 * @return true if file is valid, false if there are formatting issues
 */
static bool _validate_file(const char *filename, char *error_message, size_t error_size) {
    FILE *fp;
    ar_file_result_t result = ar_io__open_file(filename, "r", &fp);

    if (result == AR_FILE_RESULT__ERROR_NOT_FOUND) {
        snprintf(error_message, error_size, "Methodology file %s not found", filename);
        return false;
    } else if (result != AR_FILE_RESULT__SUCCESS) {
        snprintf(error_message, error_size, "Failed to open methodology file: %s",
                ar_io__error_message(result));
        return false;
    }

    // Read and validate method count
    char line[256] = {0};
    if (!ar_io__read_line(fp, line, (int)sizeof(line), filename)) {
        snprintf(error_message, error_size, "Failed to read method count from %s", filename);
        ar_io__close_file(fp, filename);
        return false;
    }

    int method_count = 0;
    char *line_endptr = NULL;
    errno = 0;
    method_count = (int)strtol(line, &line_endptr, 10);

    if (errno != 0 || line_endptr == line || (*line_endptr != '\0' && *line_endptr != '\n') ||
        method_count < 0 || method_count > MAX_METHODS) {
        snprintf(error_message, error_size, "Invalid method count in %s", filename);
        ar_io__close_file(fp, filename);
        return false;
    }

    // Validate each method entry
    for (int i = 0; i < method_count; i++) {
        // Read method name and version count line
        if (!ar_io__read_line(fp, line, (int)sizeof(line), filename)) {
            snprintf(error_message, error_size,
                    "Failed to read method entry %d from %s", i+1, filename);
            ar_io__close_file(fp, filename);
            return false;
        }

        // Count tokens - should be exactly 2 (method_name, version_count)
        char *token = NULL;
        char *next_token = NULL;
        int tokens = 0;
        char *saveptr = NULL;
        char line_copy[256];

        // Make a copy of the line since strtok_r modifies it
        ar_io__string_copy(line_copy, line, sizeof(line_copy));

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
            ar_io__close_file(fp, filename);
            return false;
        }

        // Validate method name and version count format
        char method_name[MAX_METHOD_NAME_LENGTH] = {0};
        token = strtok_r(line, " \t\n", &next_token);
        if (token == NULL) {
            snprintf(error_message, error_size,
                    "Malformed method entry - missing method name in %s for method %d",
                    filename, i+1);
            ar_io__close_file(fp, filename);
            return false;
        }

        // Copy method name with secure length check
        size_t token_len = strlen(token);
        if (token_len >= MAX_METHOD_NAME_LENGTH) {
            snprintf(error_message, error_size,
                    "Method name too long in %s for method %d", filename, i+1);
            ar_io__close_file(fp, filename);
            return false;
        }

        // Use safer string copy function
        ar_io__string_copy(method_name, token, sizeof(method_name));

        // Validate method name is not empty
        if (strlen(method_name) == 0) {
            snprintf(error_message, error_size,
                    "Empty method name in %s for method %d", filename, i+1);
            ar_io__close_file(fp, filename);
            return false;
        }

        // Validate version count exists
        token = strtok_r(NULL, " \t\n", &next_token);
        if (token == NULL) {
            snprintf(error_message, error_size,
                    "Malformed method entry - missing version count in %s for method %d",
                    filename, i+1);
            ar_io__close_file(fp, filename);
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
            ar_io__close_file(fp, filename);
            return false;
        }

        // Validate each version for this method
        for (int j = 0; j < version_count; j++) {
            // Read version line
            if (!ar_io__read_line(fp, line, (int)sizeof(line), filename)) {
                snprintf(error_message, error_size,
                        "Failed to read version for method %s (%d/%d) in %s",
                        method_name, j+1, version_count, filename);
                ar_io__close_file(fp, filename);
                return false;
            }

            // Validate version string
            if (strlen(line) == 0 || (line[0] == '\n' && strlen(line) == 1)) {
                snprintf(error_message, error_size,
                        "Empty version string for method %s (%d/%d) in %s",
                        method_name, j+1, version_count, filename);
                ar_io__close_file(fp, filename);
                return false;
            }

            // Read instructions line
            if (!ar_io__read_line(fp, line, (int)sizeof(line), filename)) {
                snprintf(error_message, error_size,
                        "Failed to read instructions for method %s (%d/%d) in %s",
                        method_name, j+1, version_count, filename);
                ar_io__close_file(fp, filename);
                return false;
            }

            // Validate instructions are not empty
            if (strlen(line) == 0 || (line[0] == '\n' && strlen(line) == 1)) {
                snprintf(error_message, error_size,
                        "Empty instructions for method %s (%d/%d) in %s",
                        method_name, j+1, version_count, filename);
                ar_io__close_file(fp, filename);
                return false;
            }
        }
    }

    ar_io__close_file(fp, filename);
    return true;
}

/* Constants */
#define MAX_INSTRUCTIONS_LENGTH 16384

/* Global State - REMOVED: Now using global instance exclusively */

/* Methodology instance structure */
struct ar_methodology_s {
    ar_log_t *ref_log;          /* Borrowed reference to log instance */
    ar_method_t ***own_methods;    /* Dynamic 2D array for instance storage */
    int *own_method_counts;     /* Dynamic array of method counts per name */
    int method_name_count;      /* Number of unique method names */
    int max_methods;            /* Current capacity for method names */
};

/* Global default instance for backward compatibility */
static ar_methodology_t *g_default_methodology = NULL;

/**
 * Get or create the global methodology instance
 * @return The global methodology instance (borrowed reference)
 */
static ar_methodology_t* _get_global_instance(void) {
    if (!g_default_methodology) {
        g_default_methodology = ar_methodology__create(NULL);
    }
    return g_default_methodology;
}

/**
 * Grow the capacity of methodology arrays when needed
 * @param mut_methodology The methodology instance to grow
 * @return true on success, false on allocation failure
 */
static bool _grow_instance_storage(ar_methodology_t *mut_methodology) {
    int new_capacity = mut_methodology->max_methods * 2;
    
    // Reallocate methods array
    ar_method_t ***new_methods = AR__HEAP__MALLOC(
        sizeof(ar_method_t**) * (size_t)new_capacity,
        "methodology methods array (grown)"
    );
    if (!new_methods) {
        return false;
    }
    
    // Reallocate method counts array
    int *new_counts = AR__HEAP__MALLOC(
        sizeof(int) * (size_t)new_capacity,
        "methodology method counts (grown)"
    );
    if (!new_counts) {
        AR__HEAP__FREE(new_methods);
        return false;
    }
    
    // Copy existing data
    for (int i = 0; i < mut_methodology->method_name_count; i++) {
        new_methods[i] = mut_methodology->own_methods[i];
        new_counts[i] = mut_methodology->own_method_counts[i];
    }
    
    // Initialize new slots
    for (int i = mut_methodology->method_name_count; i < new_capacity; i++) {
        new_methods[i] = NULL;
        new_counts[i] = 0;
    }
    
    // Free old arrays and update pointers
    AR__HEAP__FREE(mut_methodology->own_methods);
    AR__HEAP__FREE(mut_methodology->own_method_counts);
    
    mut_methodology->own_methods = new_methods;
    mut_methodology->own_method_counts = new_counts;
    mut_methodology->max_methods = new_capacity;
    
    return true;
}

/**
 * Initialize dynamic arrays for a methodology instance
 * @param mut_methodology The methodology instance to initialize
 * @param initial_capacity Initial capacity for method names
 * @return true on success, false on allocation failure
 */
static bool _initialize_instance_storage(ar_methodology_t *mut_methodology, int initial_capacity) {
    mut_methodology->max_methods = initial_capacity;
    mut_methodology->method_name_count = 0;
    
    // Allocate the methods array (array of method name pointers)
    mut_methodology->own_methods = AR__HEAP__MALLOC(
        sizeof(ar_method_t**) * (size_t)mut_methodology->max_methods, 
        "methodology methods array"
    );
    if (!mut_methodology->own_methods) {
        return false;
    }
    
    // Allocate the method counts array
    mut_methodology->own_method_counts = AR__HEAP__MALLOC(
        sizeof(int) * (size_t)mut_methodology->max_methods,
        "methodology method counts"
    );
    if (!mut_methodology->own_method_counts) {
        AR__HEAP__FREE(mut_methodology->own_methods);
        mut_methodology->own_methods = NULL;
        return false;
    }
    
    // Initialize arrays
    for (int i = 0; i < mut_methodology->max_methods; i++) {
        mut_methodology->own_methods[i] = NULL;
        mut_methodology->own_method_counts[i] = 0;
    }
    
    return true;
}

/**
 * Creates a new method object and registers it with the methodology module
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return true if method was created and registered successfully, false otherwise
 * @note Ownership: This function creates and takes ownership of the method.
 *       The caller should not worry about destroying the method.
 */
bool ar_methodology__create_method(const char *ref_name, const char *ref_instructions, 
                              const char *ref_version) {
    ar_methodology_t *mut_instance = _get_global_instance();
    if (!mut_instance) {
        return false;
    }
    
    return ar_methodology__create_method_with_instance(mut_instance, ref_name, ref_instructions, ref_version);
}


/* Instance-based Method Search Functions */
static int _find_method_idx_in_instance(ar_methodology_t *ref_methodology, const char *ref_name) {
    if (!ref_methodology) return -1;
    
    for (int i = 0; i < ref_methodology->method_name_count; i++) {
        if (ref_methodology->own_methods[i] && ref_methodology->own_methods[i][0] != NULL && 
            strcmp(ar_method__get_name(ref_methodology->own_methods[i][0]), ref_name) == 0) {
            return i;
        }
    }
    
    return -1;
}

static ar_method_t* _find_latest_method_in_instance(ar_methodology_t *ref_methodology, const char *ref_name) {
    int method_idx = _find_method_idx_in_instance(ref_methodology, ref_name);
    if (method_idx < 0 || ref_methodology->own_method_counts[method_idx] == 0) {
        return NULL;
    }
    
    // Use semantic versioning to find the latest version
    int latest_idx = 0;
    for (int i = 1; i < ref_methodology->own_method_counts[method_idx]; i++) {
        if (ref_methodology->own_methods[method_idx][i] != NULL && 
            ref_methodology->own_methods[method_idx][latest_idx] != NULL &&
            ar_semver__compare(
                ar_method__get_version(ref_methodology->own_methods[method_idx][i]),
                ar_method__get_version(ref_methodology->own_methods[method_idx][latest_idx])
            ) > 0) {
            latest_idx = i;
        }
    }
    
    if (latest_idx >= 0 && ref_methodology->own_methods[method_idx][latest_idx] != NULL) {
        return ref_methodology->own_methods[method_idx][latest_idx];
    }
    
    return NULL;
}

static ar_method_t* _find_method_in_instance(ar_methodology_t *ref_methodology, const char *ref_name, const char *ref_version) {
    int method_idx = _find_method_idx_in_instance(ref_methodology, ref_name);
    if (method_idx < 0) {
        return NULL;
    }
    
    // Case 1: If version is NULL, return the latest method
    if (ref_version == NULL) {
        return _find_latest_method_in_instance(ref_methodology, ref_name);
    }
    
    // Case 2: Exact version match
    for (int i = 0; i < ref_methodology->own_method_counts[method_idx]; i++) {
        if (ref_methodology->own_methods[method_idx][i] != NULL && 
            strcmp(ar_method__get_version(ref_methodology->own_methods[method_idx][i]), ref_version) == 0) {
            return ref_methodology->own_methods[method_idx][i];
        }
    }
    
    return NULL;
}

/* Old static array functions removed - using instance-based versions */



static void _set_method_storage_in_instance(ar_methodology_t *mut_methodology, int method_idx, int version_idx, ar_method_t *ref_method) {
    AR_ASSERT(method_idx >= 0 && method_idx < mut_methodology->method_name_count, "Method index out of bounds");
    AR_ASSERT(version_idx >= 0, "Version index out of bounds");
    
    // Check if methods array is initialized
    if (!mut_methodology->own_methods) {
        return;
    }
    
    // Allocate versions array if needed
    if (!mut_methodology->own_methods[method_idx]) {
        mut_methodology->own_methods[method_idx] = AR__HEAP__MALLOC(
            sizeof(ar_method_t*) * MAX_VERSIONS_PER_METHOD,
            "method versions array"
        );
        if (!mut_methodology->own_methods[method_idx]) {
            return;
        }
        // Initialize all slots to NULL
        for (int i = 0; i < MAX_VERSIONS_PER_METHOD; i++) {
            mut_methodology->own_methods[method_idx][i] = NULL;
        }
    }
    
    // If there's already a method at this location, destroy it first
    if (mut_methodology->own_methods[method_idx][version_idx] != NULL) {
        ar_method__destroy(mut_methodology->own_methods[method_idx][version_idx]);
    }
    
    // Store the new method
    mut_methodology->own_methods[method_idx][version_idx] = ref_method;
}


// Functions removed - were exposing internal state

// Main method access function
ar_method_t* ar_methodology__get_method(const char *ref_name, const char *ref_version) {
    ar_methodology_t *ref_instance = _get_global_instance();
    if (!ref_instance) {
        return NULL;
    }
    
    return ar_methodology__get_method_with_instance(ref_instance, ref_name, ref_version);
}

bool ar_methodology__save_methods(void) {
    ar_methodology_t *ref_instance = _get_global_instance();
    if (!ref_instance) {
        return false;
    }
    
    // Create a temporary file first, then rename it to avoid data corruption
    const char *temp_filename = METHODOLOGY_FILE_NAME ".tmp";
    
    // Use restricted mode "w" to ensure we only create/write, not execute
    FILE *mut_fp;
    ar_file_result_t result = ar_io__open_file(temp_filename, "w", &mut_fp);
    if (result != AR_FILE_RESULT__SUCCESS) {
        ar_io__error("Could not open %s for writing: %s",
                temp_filename, ar_io__error_message(result));
        return false;
    }
    
    // Set secure file permissions for the file (restrict to owner read/write)
    #ifdef _WIN32
    _chmod(temp_filename, _S_IREAD | _S_IWRITE);
    #else
    chmod(temp_filename, S_IRUSR | S_IWUSR);
    #endif
    
    // Buffer for formatted output
    char buffer[BUFFER_SIZE];
    
    // Write method count
    int written = snprintf(buffer, BUFFER_SIZE, "%d\n", ref_instance->method_name_count);
    if (written < 0 || written >= (int)BUFFER_SIZE) {
        ar_io__error("Buffer too small for method count in %s", temp_filename);
        ar_io__close_file(mut_fp, temp_filename);
        remove(temp_filename);
        return false;
    }
    
    // Write the count to the file with error checking
    if (fputs(buffer, mut_fp) == EOF) {
        ar_io__error("Failed to write method count to %s", temp_filename);
        ar_io__close_file(mut_fp, temp_filename);
        remove(temp_filename);
        return false;
    }
    
    // For each method type
    for (int i = 0; i < ref_instance->method_name_count; i++) {
        if (!ref_instance->own_methods[i] || ref_instance->own_methods[i][0] == NULL) {
            continue; // Skip empty method entries
        }
        
        // Get method name with validation
        const char *method_name = ar_method__get_name(ref_instance->own_methods[i][0]);
        if (!method_name) {
            ar_io__error("NULL method name at index %d", i);
            ar_io__close_file(mut_fp, temp_filename);
            remove(temp_filename);
            return false;
        }
        
        // Write method name and version count
        written = snprintf(buffer, BUFFER_SIZE, "%s %d\n", method_name, ref_instance->own_method_counts[i]);
        if (written < 0 || written >= (int)BUFFER_SIZE) {
            ar_io__error("Buffer too small for method data in %s", temp_filename);
            ar_io__close_file(mut_fp, temp_filename);
            remove(temp_filename);
            return false;
        }
        
        // Write the data to the file with error checking
        if (fputs(buffer, mut_fp) == EOF) {
            ar_io__error("Failed to write method data to %s", temp_filename);
            ar_io__close_file(mut_fp, temp_filename);
            remove(temp_filename);
            return false;
        }
        
        // For each version of this method
        for (int j = 0; j < ref_instance->own_method_counts[i]; j++) {
            if (ref_instance->own_methods[i][j] == NULL) {
                continue; // Skip NULL entries
            }
            
            const ar_method_t *ref_method = ref_instance->own_methods[i][j];
            
            // Get version string with validation
            const char *version = ar_method__get_version(ref_method);
            if (!version) {
                ar_io__error("NULL version for method %s at index %d", method_name, j);
                ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
                remove(METHODOLOGY_FILE_NAME);
                return false;
            }
            
            // Write version string
            written = snprintf(buffer, BUFFER_SIZE, "%s\n", version);
            if (written < 0 || written >= (int)BUFFER_SIZE) {
                ar_io__error("Buffer too small for version data in %s", temp_filename);
                ar_io__close_file(mut_fp, temp_filename);
                remove(temp_filename);
                return false;
            }
            
            // Write the version to the file with error checking
            if (fputs(buffer, mut_fp) == EOF) {
                ar_io__error("Failed to write version data to %s", temp_filename);
                ar_io__close_file(mut_fp, temp_filename);
                remove(temp_filename);
                return false;
            }
            
            // Get instructions with validation
            const char *instructions = ar_method__get_instructions(ref_method);
            if (!instructions) {
                ar_io__error("NULL instructions for method %s version %s",
                        method_name, version);
                ar_io__close_file(mut_fp, temp_filename);
                remove(temp_filename);
                return false;
            }
            
            // Check if instructions will fit in buffer
            size_t instr_len = strlen(instructions);
            if (instr_len >= BUFFER_SIZE - 2) { // -2 for newline and null terminator
                ar_io__error("Instructions too large for buffer (size: %zu) for method %s version %s",
                        instr_len, method_name, version);
                ar_io__close_file(mut_fp, temp_filename);
                remove(temp_filename);
                return false;
            }
            
            // Write instructions - these can be large, so buffer size is critical
            written = snprintf(buffer, BUFFER_SIZE, "%s\n", instructions);
            if (written < 0 || written >= (int)BUFFER_SIZE) {
                ar_io__error("Buffer overflow writing instructions for method %s version %s",
                        method_name, version);
                ar_io__close_file(mut_fp, temp_filename);
                remove(temp_filename);
                return false;
            }
            
            // Write the instructions to the file with error checking
            if (fputs(buffer, mut_fp) == EOF) {
                ar_io__error("Failed to write instructions to %s", temp_filename);
                ar_io__close_file(mut_fp, temp_filename);
                remove(temp_filename);
                return false;
            }
        }
    }
    
    // Flush and close the file
    if (fflush(mut_fp) != 0) {
        ar_io__error("Failed to flush data to %s", temp_filename);
        ar_io__close_file(mut_fp, temp_filename);
        remove(temp_filename);
        return false;
    }

    ar_io__close_file(mut_fp, temp_filename);
    
    // Rename the temporary file to the permanent file
    if (rename(temp_filename, METHODOLOGY_FILE_NAME) != 0) {
        ar_io__error("Failed to rename %s to %s: %s", temp_filename, METHODOLOGY_FILE_NAME, strerror(errno));
        remove(temp_filename);
        return false;
    }

    // Set secure permissions on the permanent file
    result = ar_io__set_secure_permissions(METHODOLOGY_FILE_NAME);
    if (result != AR_FILE_RESULT__SUCCESS) {
        ar_io__warning("Failed to set secure permissions on %s: %s",
                 METHODOLOGY_FILE_NAME, ar_io__error_message(result));
        // Continue despite permission issues - file was saved successfully
    }
    
    return true;
}

void ar_methodology__cleanup(void) {
    // Clean up the global instance if it exists
    if (g_default_methodology) {
        ar_methodology__destroy(g_default_methodology);
        g_default_methodology = NULL;
    }
}

void ar_methodology__register_method(ar_method_t *own_method) {
    ar_methodology_t *mut_instance = _get_global_instance();
    if (!mut_instance) {
        if (own_method) {
            ar_method__destroy(own_method);
        }
        return;
    }
    
    ar_methodology__register_method_with_instance(mut_instance, own_method);
}

bool ar_methodology__load_methods(void) {
    ar_methodology_t *mut_instance = _get_global_instance();
    if (!mut_instance) {
        return false;
    }
    
    // First validate the methodology file format
    char error_message[512];
    if (!_validate_file(METHODOLOGY_FILE_NAME, error_message, sizeof(error_message))) {
        if (strstr(error_message, "not found") != NULL) {
            // Not an error, might be first run
            return true;
        }

        // File exists but has errors
        ar_io__error("Methodology file validation failed: %s", error_message);

        // Create a backup and remove the corrupted file
        ar_io__warning("Creating backup of corrupted methodology file");
        ar_io__create_backup(METHODOLOGY_FILE_NAME);
        remove(METHODOLOGY_FILE_NAME);

        return true; // Return success but with empty state
    }

    // Securely open the file with proper error handling (now that we know it's valid)
    FILE *mut_fp;
    ar_file_result_t result = ar_io__open_file(METHODOLOGY_FILE_NAME, "r", &mut_fp);

    if (result != AR_FILE_RESULT__SUCCESS) {
        ar_io__error("Failed to open methodology file: %s", ar_io__error_message(result));
        return false;
    }

    // Verify file permissions (should be readable only by owner)
    result = ar_io__set_secure_permissions(METHODOLOGY_FILE_NAME);
    if (result != AR_FILE_RESULT__SUCCESS) {
        ar_io__warning("Failed to set secure permissions on methodology file: %s",
                   ar_io__error_message(result));
        // Continue anyway with a warning
    }
    
    // Initialize line buffer securely
    char line[LINE_SIZE] = {0}; // Initialize to zeros for security
    
    // Read the first line to get method count with error checking
    if (!ar_io__read_line(mut_fp, line, (int)LINE_SIZE, METHODOLOGY_FILE_NAME)) {
        ar_io__error("Failed to read method count from %s", METHODOLOGY_FILE_NAME);
        ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);

        // Create a backup and remove the corrupted file
        ar_io__warning("Creating backup of corrupted methodology file");
        ar_io__create_backup(METHODOLOGY_FILE_NAME);
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
        ar_io__error("Invalid method count in %s", METHODOLOGY_FILE_NAME);
        ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);

        // Create a backup and delete the corrupted file
        ar_io__warning("Creating backup of corrupted methodology file");
        ar_io__create_backup(METHODOLOGY_FILE_NAME);
        remove(METHODOLOGY_FILE_NAME);
        return true;
    }
    
    // Clear existing methods in the instance
    if (mut_instance->own_methods) {
        for (int i = 0; i < mut_instance->method_name_count; i++) {
            if (mut_instance->own_methods[i]) {
                for (int j = 0; j < mut_instance->own_method_counts[i]; j++) {
                    if (mut_instance->own_methods[i][j]) {
                        ar_method__destroy(mut_instance->own_methods[i][j]);
                        mut_instance->own_methods[i][j] = NULL;
                    }
                }
                AR__HEAP__FREE(mut_instance->own_methods[i]);
                mut_instance->own_methods[i] = NULL;
            }
            mut_instance->own_method_counts[i] = 0;
        }
    }
    mut_instance->method_name_count = 0;
    
    // Process each method type
    for (int i = 0; i < method_count; i++) {
        char name[MAX_METHOD_NAME_LENGTH] = {0}; // Initialize to zeros for security
        int version_count = 0;
        
        // Read the next line for method name and version count
        if (fgets(line, (int)LINE_SIZE, mut_fp) == NULL) {
            ar_io__error("Unexpected end of file in %s (method %d)",
                    METHODOLOGY_FILE_NAME, i+1);
            ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
            // Delete the corrupted file and start fresh
            ar_io__warning("Deleting corrupted methodology file");
            remove(METHODOLOGY_FILE_NAME);
            return true;
        }
        
        // Parse the method name and version count manually using strtok_r
        char *token = NULL;
        char *next_token = NULL;
        
        // Get method name
        token = strtok_r(line, " \t\n", &next_token);
        if (token == NULL) {
            ar_io__error("Malformed method entry - missing name in %s (method %d)",
                    METHODOLOGY_FILE_NAME, i+1);
            ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
            ar_io__warning("Deleting corrupted methodology file");
            remove(METHODOLOGY_FILE_NAME);
            return true;
        }
        
        // Copy method name with secure length check
        size_t token_len = strlen(token);
        if (token_len >= MAX_METHOD_NAME_LENGTH) {
            ar_io__error("Method name too long in %s (method %d)",
                    METHODOLOGY_FILE_NAME, i+1);
            ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
            ar_io__warning("Deleting corrupted methodology file");
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
            ar_io__error("Malformed method entry - missing version count in %s (method %d)",
                    METHODOLOGY_FILE_NAME, i+1);
            ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
            ar_io__warning("Deleting corrupted methodology file");
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
            ar_io__error("Invalid version count for method %s in %s",
                    name, METHODOLOGY_FILE_NAME);
            ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
            ar_io__warning("Deleting corrupted methodology file");
            remove(METHODOLOGY_FILE_NAME);
            return true;
        }
        
        // Add this method to the instance
        int method_idx = mut_instance->method_name_count++;
        if (method_idx >= mut_instance->max_methods) {
            // Need to grow the arrays
            if (!_grow_instance_storage(mut_instance)) {
                ar_io__error("Failed to grow methodology storage while loading");
                ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
                return false;
            }
        }
        mut_instance->own_method_counts[method_idx] = 0; // Reset count for this method
        
        // For each version of this method
        for (int j = 0; j < version_count; j++) {
            // Verify we don't exceed the maximum versions per method
            if (mut_instance->own_method_counts[method_idx] >= MAX_VERSIONS_PER_METHOD) {
                ar_io__error("Maximum versions reached for method %s in %s",
                        name, METHODOLOGY_FILE_NAME);
                ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
                return false;
            }
            
            // Buffer for version string, initialized to zeros
            char version[64] = {0}; 
            
            // Read version string with error checking
            if (fgets(line, (int)LINE_SIZE, mut_fp) == NULL) {
                ar_io__error("Unexpected end of file in %s when reading version for method %s",
                        METHODOLOGY_FILE_NAME, name);
                ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
                ar_io__warning("Deleting corrupted methodology file");
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
                ar_io__error("Empty version string for method %s in %s",
                        name, METHODOLOGY_FILE_NAME);
                ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
                ar_io__warning("Deleting corrupted methodology file");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Copy the version string securely
            if (len > sizeof(version) - 1) {
                ar_io__error("Version string too long for method %s in %s",
                        name, METHODOLOGY_FILE_NAME);
                ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
                ar_io__warning("Deleting corrupted methodology file");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Use safer string copy function
            ar_io__string_copy(version, line, sizeof(version));
            
            // Buffer for instructions with security initialization
            char instructions[MAX_INSTRUCTIONS_LENGTH] = {0};
            
            // Read the instructions with error checking
            if (fgets(instructions, MAX_INSTRUCTIONS_LENGTH, mut_fp) == NULL) {
                ar_io__error("Could not read instructions for method %s version %s in %s",
                       name, version, METHODOLOGY_FILE_NAME);
                ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
                ar_io__warning("Deleting corrupted methodology file");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Validate instructions
            size_t instr_len = strlen(instructions);
            if (instr_len == 0) {
                ar_io__error("Empty instructions for method %s version %s in %s",
                        name, version, METHODOLOGY_FILE_NAME);
                ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
                ar_io__warning("Deleting corrupted methodology file");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Remove trailing newline if present
            if (instructions[instr_len-1] == '\n') {
                instructions[instr_len-1] = '\0';
            }
            
            // Check for truncation (buffer filled completely)
            if (instr_len >= MAX_INSTRUCTIONS_LENGTH - 1) {
                ar_io__warning("Instructions for method %s version %s may be truncated in %s",
                        name, version, METHODOLOGY_FILE_NAME);
            }
            
            // Create a new method with data from the file
            ar_method_t *own_method = ar_method__create_with_log(name, instructions, version, mut_instance->ref_log);
            
            if (own_method) {
                // Store the method in the instance
                _set_method_storage_in_instance(mut_instance, method_idx, mut_instance->own_method_counts[method_idx]++, own_method);
                own_method = NULL; // Mark as transferred
            } else {
                ar_io__error("Failed to create method %s version %s", name, version);
                ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
                return false;
            }
        }
    }
    
    ar_io__close_file(mut_fp, METHODOLOGY_FILE_NAME);
    return true;
}

/**
 * Unregister a method from the methodology
 * @param ref_name Method name (borrowed reference)
 * @param ref_version Version string of the method to unregister
 * @return true if method was successfully unregistered, false otherwise
 * @note This will fail if there are active agents using this method
 */
bool ar_methodology__unregister_method(const char *ref_name, const char *ref_version) {
    ar_methodology_t *mut_instance = _get_global_instance();
    if (!mut_instance) {
        return false;
    }
    
    return ar_methodology__unregister_method_with_instance(mut_instance, ref_name, ref_version);
}

ar_methodology_t* ar_methodology__create(ar_log_t *ref_log) {
    // Allocate memory for the methodology instance
    ar_methodology_t *own_methodology = AR__HEAP__MALLOC(sizeof(ar_methodology_t), "ar_methodology_t");
    if (!own_methodology) {
        return NULL;
    }
    
    // Store the log reference
    own_methodology->ref_log = ref_log;
    
    // Initialize instance storage with initial capacity of 10 method names
    if (!_initialize_instance_storage(own_methodology, 10)) {
        AR__HEAP__FREE(own_methodology);
        return NULL;
    }
    
    return own_methodology;
}

/**
 * Destroy a methodology instance
 * @param own_methodology The methodology instance to destroy (takes ownership)
 * @note Ownership: Takes ownership and destroys the instance
 */
void ar_methodology__destroy(ar_methodology_t *own_methodology) {
    if (own_methodology) {
        // Clean up all methods in instance storage
        if (own_methodology->own_methods) {
            for (int i = 0; i < own_methodology->method_name_count; i++) {
                if (own_methodology->own_methods[i]) {
                    // Free all versions for this method name
                    for (int j = 0; j < own_methodology->own_method_counts[i]; j++) {
                        if (own_methodology->own_methods[i][j]) {
                            ar_method__destroy(own_methodology->own_methods[i][j]);
                        }
                    }
                    // Free the versions array for this method name
                    AR__HEAP__FREE(own_methodology->own_methods[i]);
                }
            }
            // Free the methods array itself
            AR__HEAP__FREE(own_methodology->own_methods);
        }
        
        // Free the method counts array
        if (own_methodology->own_method_counts) {
            AR__HEAP__FREE(own_methodology->own_method_counts);
        }
        
        // Free the instance
        AR__HEAP__FREE(own_methodology);
    }
}

/* Instance-aware versions of public functions */

ar_method_t* ar_methodology__get_method_with_instance(ar_methodology_t *ref_methodology, 
                                                   const char *ref_name, 
                                                   const char *ref_version) {
    if (!ref_methodology) {
        return NULL;
    }
    return _find_method_in_instance(ref_methodology, ref_name, ref_version);
}

void ar_methodology__register_method_with_instance(ar_methodology_t *mut_methodology, 
                                                   ar_method_t *own_method) {
    if (!mut_methodology || !own_method) {
        if (own_method) {
            ar_method__destroy(own_method);
        }
        return;
    }
    
    const char *method_name = ar_method__get_name(own_method);
    const char *method_version = ar_method__get_version(own_method);
    
    // Find or create a method index for this name
    int method_idx = _find_method_idx_in_instance(mut_methodology, method_name);
    if (method_idx < 0) {
        // No existing method with this name, create a new entry
        if (mut_methodology->method_name_count >= mut_methodology->max_methods) {
            // Need to grow the arrays
            if (!_grow_instance_storage(mut_methodology)) {
                ar_io__error("Failed to grow methodology storage");
                ar_method__destroy(own_method);
                return;
            }
        }
        
        method_idx = mut_methodology->method_name_count++;
        // Initialize the method count for the new entry
        mut_methodology->own_method_counts[method_idx] = 0;
    }
    
    // Check if we've reached max versions for this method
    if (mut_methodology->own_method_counts[method_idx] >= MAX_VERSIONS_PER_METHOD) {
        ar_io__error("Maximum number of versions reached for method %s", method_name);
        ar_method__destroy(own_method); // Clean up the method
        return;
    }
    
    // Check for version conflicts
    bool version_conflict = false;
    if (mut_methodology->own_methods[method_idx]) {
        for (int i = 0; i < mut_methodology->own_method_counts[method_idx]; i++) {
            if (mut_methodology->own_methods[method_idx][i] != NULL && 
                strcmp(ar_method__get_version(mut_methodology->own_methods[method_idx][i]), method_version) == 0) {
                version_conflict = true;
                break;
            }
        }
    }
    
    if (version_conflict) {
        ar_io__error("Method %s version %s already exists", method_name, method_version);
        ar_method__destroy(own_method); // Clean up the method
        return;
    }
    
    // Allocate or expand the versions array if needed
    if (!mut_methodology->own_methods[method_idx]) {
        mut_methodology->own_methods[method_idx] = AR__HEAP__MALLOC(
            sizeof(ar_method_t*) * MAX_VERSIONS_PER_METHOD,
            "method versions array"
        );
        if (!mut_methodology->own_methods[method_idx]) {
            ar_io__error("Failed to allocate memory for method versions");
            ar_method__destroy(own_method);
            return;
        }
        // Initialize all pointers to NULL
        for (int i = 0; i < MAX_VERSIONS_PER_METHOD; i++) {
            mut_methodology->own_methods[method_idx][i] = NULL;
        }
        mut_methodology->own_method_counts[method_idx] = 0;
    }
    
    // Add the method to the versions array
    int version_idx = mut_methodology->own_method_counts[method_idx];
    mut_methodology->own_methods[method_idx][version_idx] = own_method;
    mut_methodology->own_method_counts[method_idx]++;
}

bool ar_methodology__create_method_with_instance(ar_methodology_t *mut_methodology,
                                                 const char *ref_name, 
                                                 const char *ref_instructions,
                                                 const char *ref_version) {
    if (!mut_methodology || !ref_name || !ref_instructions || !ref_version) {
        return false;
    }
    
    // Pass the ar_log from the methodology instance to ar_method__create
    ar_method_t *own_method = ar_method__create_with_log(ref_name, ref_instructions, ref_version, mut_methodology->ref_log);
    if (!own_method) {
        return false;
    }
    
    ar_methodology__register_method_with_instance(mut_methodology, own_method);
    // ownership transferred
    
    return true;
}

bool ar_methodology__save_methods_with_instance(ar_methodology_t *ref_methodology, 
                                                const char *ref_filename) {
    if (!ref_methodology || !ref_filename) {
        return false;
    }
    
    FILE *fp;
    ar_file_result_t result = ar_io__open_file(ref_filename, "w", &fp);
    if (result != AR_FILE_RESULT__SUCCESS) {
        ar_io__error("Failed to open file %s for writing: %s", ref_filename, 
                    ar_io__error_message(result));
        return false;
    }
    
    // Buffer for formatted output
    char buffer[BUFFER_SIZE];
    
    // Write method count (count of unique method names)
    int written = snprintf(buffer, BUFFER_SIZE, "%d\n", ref_methodology->method_name_count);
    if (written < 0 || written >= (int)BUFFER_SIZE) {
        ar_io__error("Buffer too small for method count in %s", ref_filename);
        ar_io__close_file(fp, ref_filename);
        return false;
    }
    
    if (fputs(buffer, fp) == EOF) {
        ar_io__error("Failed to write method count to %s", ref_filename);
        ar_io__close_file(fp, ref_filename);
        return false;
    }
    
    // Save each method type
    for (int i = 0; i < ref_methodology->method_name_count; i++) {
        if (!ref_methodology->own_methods[i] || ref_methodology->own_method_counts[i] == 0) {
            continue;
        }
        
        // Get method name from the first non-null method
        const char *method_name = NULL;
        for (int j = 0; j < ref_methodology->own_method_counts[i]; j++) {
            if (ref_methodology->own_methods[i][j]) {
                method_name = ar_method__get_name(ref_methodology->own_methods[i][j]);
                break;
            }
        }
        
        if (!method_name) {
            continue;
        }
        
        // Write method name and version count
        written = snprintf(buffer, BUFFER_SIZE, "%s %d\n", method_name, ref_methodology->own_method_counts[i]);
        if (written < 0 || written >= (int)BUFFER_SIZE || fputs(buffer, fp) == EOF) {
            ar_io__error("Failed to write method header to %s", ref_filename);
            ar_io__close_file(fp, ref_filename);
            return false;
        }
        
        // Write each version
        for (int j = 0; j < ref_methodology->own_method_counts[i]; j++) {
            ar_method_t *method = ref_methodology->own_methods[i][j];
            if (!method) continue;
            
            const char *version = ar_method__get_version(method);
            const char *instructions = ar_method__get_instructions(method);
            
            // Write version
            written = snprintf(buffer, BUFFER_SIZE, "%s\n", version);
            if (written < 0 || written >= (int)BUFFER_SIZE || fputs(buffer, fp) == EOF) {
                ar_io__error("Failed to write version to %s", ref_filename);
                ar_io__close_file(fp, ref_filename);
                return false;
            }
            
            // Write instructions
            written = snprintf(buffer, BUFFER_SIZE, "%s\n", instructions ? instructions : "");
            if (written < 0 || written >= (int)BUFFER_SIZE || fputs(buffer, fp) == EOF) {
                ar_io__error("Failed to write instructions to %s", ref_filename);
                ar_io__close_file(fp, ref_filename);
                return false;
            }
        }
    }
    
    ar_io__close_file(fp, ref_filename);
    return true;
}

bool ar_methodology__load_methods_with_instance(ar_methodology_t *mut_methodology, 
                                                const char *ref_filename) {
    if (!mut_methodology || !ref_filename) {
        return false;
    }
    
    FILE *fp;
    ar_file_result_t result = ar_io__open_file(ref_filename, "r", &fp);
    if (result != AR_FILE_RESULT__SUCCESS) {
        if (result != AR_FILE_RESULT__ERROR_NOT_FOUND) {
            ar_io__error("Failed to open file %s for reading: %s", ref_filename,
                        ar_io__error_message(result));
        }
        return false;
    }
    
    char line[LINE_SIZE] = {0};
    char *end_ptr = NULL;
    
    // Read method count
    if (!ar_io__read_line(fp, line, (int)sizeof(line), ref_filename)) {
        ar_io__close_file(fp, ref_filename);
        return false;
    }
    
    errno = 0;
    long method_count = strtol(line, &end_ptr, 10);
    if (errno != 0 || end_ptr == line || method_count < 0 || method_count > MAX_METHODS) {
        ar_io__error("Invalid method count in %s: %s", ref_filename, line);
        ar_io__close_file(fp, ref_filename);
        return false;
    }
    
    // Load each method
    for (int i = 0; i < method_count; i++) {
        // Read method name and version count
        if (!ar_io__read_line(fp, line, (int)sizeof(line), ref_filename)) {
            ar_io__error("Failed to read method entry %d from %s", i+1, ref_filename);
            ar_io__close_file(fp, ref_filename);
            return false;
        }
        
        char method_name[MAX_METHOD_NAME_LENGTH] = {0};
        int version_count = 0;
        if (sscanf(line, "%255s %d", method_name, &version_count) != 2) {
            ar_io__error("Malformed method entry in %s: %s", ref_filename, line);
            ar_io__close_file(fp, ref_filename);
            return false;
        }
        
        // Load each version
        for (int v = 0; v < version_count; v++) {
            // Read version
            if (!ar_io__read_line(fp, line, (int)sizeof(line), ref_filename)) {
                ar_io__error("Failed to read version for method %s", method_name);
                ar_io__close_file(fp, ref_filename);
                return false;
            }
            char *version = AR__HEAP__STRDUP(ar_string__trim(line), "method version");
            if (!version) {
                ar_io__close_file(fp, ref_filename);
                return false;
            }
            
            // Read instructions
            if (!ar_io__read_line(fp, line, (int)sizeof(line), ref_filename)) {
                AR__HEAP__FREE(version);
                ar_io__close_file(fp, ref_filename);
                return false;
            }
            
            // Instructions can be empty, so we don't check for that
            // Trim to remove newline
            char *instructions = AR__HEAP__STRDUP(ar_string__trim(line), "method instructions");
            if (!instructions) {
                AR__HEAP__FREE(version);
                ar_io__close_file(fp, ref_filename);
                return false;
            }
            
            // Create and register the method
            ar_method_t *method = ar_method__create_with_log(method_name, instructions, version, mut_methodology->ref_log);
            AR__HEAP__FREE(version);
            AR__HEAP__FREE(instructions);
            
            if (method) {
                ar_methodology__register_method_with_instance(mut_methodology, method);
            }
        }
    }
    
    ar_io__close_file(fp, ref_filename);
    return true;
}

bool ar_methodology__unregister_method_with_instance(ar_methodology_t *mut_methodology,
                                                     const char *ref_name,
                                                     const char *ref_version) {
    if (!mut_methodology || !ref_name || !ref_version) {
        return false;
    }
    
    // Find the method index
    int method_idx = _find_method_idx_in_instance(mut_methodology, ref_name);
    if (method_idx < 0) {
        return false; // Method name not found
    }
    
    // Find the version index
    if (!mut_methodology->own_methods[method_idx]) {
        return false;
    }
    
    int version_idx = -1;
    for (int i = 0; i < mut_methodology->own_method_counts[method_idx]; i++) {
        ar_method_t *method = mut_methodology->own_methods[method_idx][i];
        if (method && strcmp(ar_method__get_version(method), ref_version) == 0) {
            version_idx = i;
            break;
        }
    }
    
    if (version_idx < 0) {
        return false; // Version not found
    }
    
    // Get the method to check if it's in use
    ar_method_t *method_to_remove = mut_methodology->own_methods[method_idx][version_idx];
    
    // Check if method is in use by any agents
    if (ar_agency__count_agents_using_method(method_to_remove) > 0) {
        ar_io__error("Cannot unregister method %s version %s: agents are still using it", 
                    ref_name, ref_version);
        return false;
    }
    
    // Destroy the method
    ar_method__destroy(mut_methodology->own_methods[method_idx][version_idx]);
    
    // Shift remaining versions down
    for (int i = version_idx; i < mut_methodology->own_method_counts[method_idx] - 1; i++) {
        mut_methodology->own_methods[method_idx][i] = mut_methodology->own_methods[method_idx][i + 1];
    }
    
    // Clear the last slot and decrement count
    mut_methodology->own_methods[method_idx][mut_methodology->own_method_counts[method_idx] - 1] = NULL;
    mut_methodology->own_method_counts[method_idx]--;
    
    // If this was the last version, we could free the versions array
    // but we'll keep it allocated for potential future use
    
    return true;
}
