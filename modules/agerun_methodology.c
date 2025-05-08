#include "agerun_methodology.h"
#include "agerun_method.h"
#include "agerun_string.h"
#include "agerun_heap.h"
#include "agerun_semver.h"
#include "agerun_agency.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

/* Constants */
#define MAX_METHODS 256
#define MAX_VERSIONS_PER_METHOD 64
#define MAX_METHOD_NAME_LENGTH 64
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
    FILE *mut_fp = fopen(temp_filename, "w");
    if (!mut_fp) {
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
    
    // Define buffer size constant for clarity and maintainability
    const size_t BUFFER_SIZE = 16384; // Increased buffer size for larger instructions
    char buffer[BUFFER_SIZE]; // Buffer for formatted output
    
    // Write method count
    int written = snprintf(buffer, BUFFER_SIZE, "%d\n", method_name_count);
    if (written < 0 || written >= (int)BUFFER_SIZE) {
        fprintf(stderr, "Error: Buffer too small for method count in %s\n", temp_filename);
        fclose(mut_fp);
        remove(temp_filename);
        return false;
    }
    
    // Write the count to the file with error checking
    if (fputs(buffer, mut_fp) == EOF) {
        fprintf(stderr, "Error: Failed to write method count to %s\n", temp_filename);
        fclose(mut_fp);
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
            fprintf(stderr, "Error: NULL method name at index %d\n", i);
            fclose(mut_fp);
            remove(temp_filename);
            return false;
        }
        
        // Write method name and version count
        written = snprintf(buffer, BUFFER_SIZE, "%s %d\n", method_name, method_counts[i]);
        if (written < 0 || written >= (int)BUFFER_SIZE) {
            fprintf(stderr, "Error: Buffer too small for method data in %s\n", temp_filename);
            fclose(mut_fp);
            remove(temp_filename);
            return false;
        }
        
        // Write the data to the file with error checking
        if (fputs(buffer, mut_fp) == EOF) {
            fprintf(stderr, "Error: Failed to write method data to %s\n", temp_filename);
            fclose(mut_fp);
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
                fprintf(stderr, "Error: NULL version for method %s at index %d\n", method_name, j);
                fclose(mut_fp);
                remove(temp_filename);
                return false;
            }
            
            // Write version string
            written = snprintf(buffer, BUFFER_SIZE, "%s\n", version);
            if (written < 0 || written >= (int)BUFFER_SIZE) {
                fprintf(stderr, "Error: Buffer too small for version data in %s\n", temp_filename);
                fclose(mut_fp);
                remove(temp_filename);
                return false;
            }
            
            // Write the version to the file with error checking
            if (fputs(buffer, mut_fp) == EOF) {
                fprintf(stderr, "Error: Failed to write version data to %s\n", temp_filename);
                fclose(mut_fp);
                remove(temp_filename);
                return false;
            }
            
            // Get instructions with validation
            const char *instructions = ar_method_get_instructions(ref_method);
            if (!instructions) {
                fprintf(stderr, "Error: NULL instructions for method %s version %s\n", 
                        method_name, version);
                fclose(mut_fp);
                remove(temp_filename);
                return false;
            }
            
            // Check if instructions will fit in buffer
            size_t instr_len = strlen(instructions);
            if (instr_len >= BUFFER_SIZE - 2) { // -2 for newline and null terminator
                fprintf(stderr, "Error: Instructions too large for buffer (size: %zu) for method %s version %s\n", 
                        instr_len, method_name, version);
                fclose(mut_fp);
                remove(temp_filename);
                return false;
            }
            
            // Write instructions - these can be large, so buffer size is critical
            written = snprintf(buffer, BUFFER_SIZE, "%s\n", instructions);
            if (written < 0 || written >= (int)BUFFER_SIZE) {
                fprintf(stderr, "Error: Buffer overflow writing instructions for method %s version %s\n", 
                        method_name, version);
                fclose(mut_fp);
                remove(temp_filename);
                return false;
            }
            
            // Write the instructions to the file with error checking
            if (fputs(buffer, mut_fp) == EOF) {
                fprintf(stderr, "Error: Failed to write instructions to %s\n", temp_filename);
                fclose(mut_fp);
                remove(temp_filename);
                return false;
            }
        }
    }
    
    // Flush and close the file
    if (fflush(mut_fp) != 0) {
        fprintf(stderr, "Error: Failed to flush data to %s\n", temp_filename);
        fclose(mut_fp);
        remove(temp_filename);
        return false;
    }
    
    fclose(mut_fp);
    
    // Rename the temporary file to the permanent file
    if (rename(temp_filename, METHODOLOGY_FILE_NAME) != 0) {
        fprintf(stderr, "Error: Failed to rename %s to %s\n", temp_filename, METHODOLOGY_FILE_NAME);
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
            printf("Error: Maximum number of method types reached\n");
            ar_method_destroy(own_method); // Clean up the method
            return;
        }
        
        method_idx = method_name_count++;
    }
    
    // Check if we've reached max versions for this method
    if (method_counts[method_idx] >= MAX_VERSIONS_PER_METHOD) {
        printf("Error: Maximum number of versions reached for method %s\n", method_name);
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
        printf("Warning: Method %s version %s already exists\n", method_name, method_version);
    }
    
    // Find the next version slot
    int version_idx = method_counts[method_idx];
    
    // Store the method in our methods array, handling any existing method
    ar_methodology_set_method_storage(method_idx, version_idx, own_method);
    method_counts[method_idx]++;
    
    printf("Registered method %s version %s\n", method_name, method_version);
    
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
                        printf("Updated %d agent(s) from method %s version %s to version %s\n",
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
    // Securely open the file with error checking
    FILE *mut_fp = fopen(METHODOLOGY_FILE_NAME, "r");
    if (!mut_fp) {
        // Not an error, might be first run
        return true;
    }
    
    // Verify file permissions (should be readable only by owner)
    struct stat file_stat;
    if (stat(METHODOLOGY_FILE_NAME, &file_stat) == 0) {
        // Check if file has secure permissions (owner read/write only)
        #ifdef _WIN32
        // Windows doesn't expose the same permission bits, but we can check if it's readonly
        if (file_stat.st_mode & _S_IREAD && !(file_stat.st_mode & _S_IWRITE)) {
            fprintf(stderr, "Warning: Methodology file is read-only\n");
        }
        #else
        // On Unix-like systems, check if file is accessible by others
        if (file_stat.st_mode & (S_IRWXG | S_IRWXO)) {
            fprintf(stderr, "Warning: Methodology file has insecure permissions\n");
            // We continue anyway, but we've warned the user
        }
        #endif
    }
    
    // Define buffer size and initialize line buffer securely
    const size_t LINE_SIZE = 256;
    char line[LINE_SIZE] = {0}; // Initialize to zeros for security
    
    // Read the first line to get method count with error checking
    if (fgets(line, LINE_SIZE, mut_fp) == NULL) {
        fprintf(stderr, "Error: Empty file %s\n", METHODOLOGY_FILE_NAME);
        fclose(mut_fp);
        // Delete the corrupted file and start fresh
        fprintf(stderr, "Deleting corrupted methodology file\n");
        remove(METHODOLOGY_FILE_NAME);
        return true;
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
        fprintf(stderr, "Error: Invalid method count in %s\n", METHODOLOGY_FILE_NAME);
        fclose(mut_fp);
        // Delete the corrupted file and start fresh
        fprintf(stderr, "Deleting corrupted methodology file\n");
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
            fprintf(stderr, "Error: Unexpected end of file in %s (method %d)\n", 
                    METHODOLOGY_FILE_NAME, i+1);
            fclose(mut_fp);
            // Delete the corrupted file and start fresh
            fprintf(stderr, "Deleting corrupted methodology file\n");
            remove(METHODOLOGY_FILE_NAME);
            return true;
        }
        
        // Parse the method name and version count manually using strtok_r
        char *token = NULL;
        char *next_token = NULL;
        
        // Get method name
        token = strtok_r(line, " \t\n", &next_token);
        if (token == NULL) {
            fprintf(stderr, "Error: Malformed method entry - missing name in %s (method %d)\n", 
                    METHODOLOGY_FILE_NAME, i+1);
            fclose(mut_fp);
            fprintf(stderr, "Deleting corrupted methodology file\n");
            remove(METHODOLOGY_FILE_NAME);
            return true;
        }
        
        // Copy method name with secure length check
        size_t token_len = strlen(token);
        if (token_len >= MAX_METHOD_NAME_LENGTH) {
            fprintf(stderr, "Error: Method name too long in %s (method %d)\n", 
                    METHODOLOGY_FILE_NAME, i+1);
            fclose(mut_fp);
            fprintf(stderr, "Deleting corrupted methodology file\n");
            remove(METHODOLOGY_FILE_NAME);
            return true;
        }
        
        // Use strncpy with explicit null termination
        strncpy(name, token, MAX_METHOD_NAME_LENGTH - 1);
        name[MAX_METHOD_NAME_LENGTH - 1] = '\0';  // Ensure null-termination
        
        // Get version count
        token = strtok_r(NULL, " \t\n", &next_token);
        if (token == NULL) {
            fprintf(stderr, "Error: Malformed method entry - missing version count in %s (method %d)\n", 
                    METHODOLOGY_FILE_NAME, i+1);
            fclose(mut_fp);
            fprintf(stderr, "Deleting corrupted methodology file\n");
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
            fprintf(stderr, "Error: Invalid version count for method %s in %s\n", 
                    name, METHODOLOGY_FILE_NAME);
            fclose(mut_fp);
            fprintf(stderr, "Deleting corrupted methodology file\n");
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
                fprintf(stderr, "Error: Maximum versions reached for method %s in %s\n", 
                        name, METHODOLOGY_FILE_NAME);
                fclose(mut_fp);
                return false;
            }
            
            // Buffer for version string, initialized to zeros
            char version[64] = {0}; 
            
            // Read version string with error checking
            if (fgets(line, LINE_SIZE, mut_fp) == NULL) {
                fprintf(stderr, "Error: Unexpected end of file in %s when reading version for method %s\n", 
                        METHODOLOGY_FILE_NAME, name);
                fclose(mut_fp);
                fprintf(stderr, "Deleting corrupted methodology file\n");
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
                fprintf(stderr, "Error: Empty version string for method %s in %s\n", 
                        name, METHODOLOGY_FILE_NAME);
                fclose(mut_fp);
                fprintf(stderr, "Deleting corrupted methodology file\n");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Copy the version string securely
            if (len > sizeof(version) - 1) {
                fprintf(stderr, "Error: Version string too long for method %s in %s\n", 
                        name, METHODOLOGY_FILE_NAME);
                fclose(mut_fp);
                fprintf(stderr, "Deleting corrupted methodology file\n");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            strncpy(version, line, sizeof(version) - 1);
            version[sizeof(version) - 1] = '\0';  // Ensure null-termination
            
            // Buffer for instructions with security initialization
            char instructions[MAX_INSTRUCTIONS_LENGTH] = {0};
            
            // Read the instructions with error checking
            if (fgets(instructions, MAX_INSTRUCTIONS_LENGTH, mut_fp) == NULL) {
                fprintf(stderr, "Error: Could not read instructions for method %s version %s in %s\n", 
                       name, version, METHODOLOGY_FILE_NAME);
                fclose(mut_fp);
                fprintf(stderr, "Deleting corrupted methodology file\n");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Validate instructions
            size_t instr_len = strlen(instructions);
            if (instr_len == 0) {
                fprintf(stderr, "Error: Empty instructions for method %s version %s in %s\n", 
                        name, version, METHODOLOGY_FILE_NAME);
                fclose(mut_fp);
                fprintf(stderr, "Deleting corrupted methodology file\n");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Remove trailing newline if present
            if (instructions[instr_len-1] == '\n') {
                instructions[instr_len-1] = '\0';
            }
            
            // Check for truncation (buffer filled completely)
            if (instr_len >= MAX_INSTRUCTIONS_LENGTH - 1) {
                fprintf(stderr, "Warning: Instructions for method %s version %s may be truncated in %s\n", 
                        name, version, METHODOLOGY_FILE_NAME);
            }
            
            // Create a new method with data from the file
            method_t *own_method = ar_method_create(name, instructions, version);
            
            if (own_method) {
                // Store the method in the methods array
                ar_methodology_set_method_storage(method_idx, method_counts[method_idx]++, own_method);
                own_method = NULL; // Mark as transferred
            } else {
                fprintf(stderr, "Error: Failed to create method %s version %s\n", name, version);
                fclose(mut_fp);
                return false;
            }
        }
    }
    
    fclose(mut_fp);
    return true;
}
