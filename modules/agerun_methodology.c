#include "agerun_methodology.h"
#include "agerun_method.h"
#include "agerun_string.h"
#include "agerun_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
    
    // For now, with string versions, we'll just return the last registered version
    // In a future implementation, we could parse semantic versions and compare them
    int latest_idx = method_counts[method_idx] - 1;
    if (latest_idx >= 0) {
        return methods[method_idx][latest_idx];
    }
    
    return NULL;
}

static method_t* find_method(const char *ref_name, const char *ref_version) {
    int method_idx = ar_methodology_find_method_idx(ref_name);
    if (method_idx < 0) {
        return NULL;
    }
    
    // Case 1: Exact version match
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i] != NULL && 
            strcmp(ar_method_get_version(methods[method_idx][i]), ref_version) == 0) {
            return methods[method_idx][i];
        }
    }
    
    // Case 2: If version is NULL, return the latest method
    if (ref_version == NULL) {
        return find_latest_method(ref_name);
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
    // Simple placeholder implementation
    FILE *mut_fp = fopen(METHODOLOGY_FILE_NAME, "w");
    if (!mut_fp) {
        printf("Error: Could not open %s for writing\n", METHODOLOGY_FILE_NAME);
        return false;
    }
    
    // Write the number of method types
    fprintf(mut_fp, "%d\n", method_name_count);
    
    // For each method type
    for (int i = 0; i < method_name_count; i++) {
        if (methods[i][0] == NULL) {
            continue;
        }
        
        // Write the method name and number of versions
        fprintf(mut_fp, "%s %d\n", ar_method_get_name(methods[i][0]), method_counts[i]);
        
        // For each version
        for (int j = 0; j < method_counts[i]; j++) {
            if (methods[i][j] == NULL) {
                continue;
            }
            
            const method_t *ref_method = methods[i][j];
            
            // Write method metadata - just version string now
            fprintf(mut_fp, "%s\n", ar_method_get_version(ref_method));
            
            // Write instructions
            fprintf(mut_fp, "%s\n", ar_method_get_instructions(ref_method));
        }
    }
    
    fclose(mut_fp);
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
    
    // Find the next version slot
    int version_idx = method_counts[method_idx];
    
    // Check for version conflicts and adjust if needed
    bool version_conflict = false;
    for (int i = 0; i < version_idx; i++) {
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
    
    // Store the method in our methods array, handling any existing method
    ar_methodology_set_method_storage(method_idx, version_idx, own_method);
    method_counts[method_idx]++;
    
    printf("Registered method %s version %s\n", method_name, method_version);
}

bool ar_methodology_load_methods(void) {
    FILE *mut_fp = fopen(METHODOLOGY_FILE_NAME, "r");
    if (!mut_fp) {
        // Not an error, might be first run
        return true;
    }
    
    // Read the number of method types
    int method_count = 0;
    if (fscanf(mut_fp, "%d", &method_count) != 1 || method_count <= 0 || method_count > MAX_METHODS) {
        printf("Error: Invalid method count in %s\n", METHODOLOGY_FILE_NAME);
        fclose(mut_fp);
        // Delete the corrupted file and start fresh
        printf("Deleting corrupted methodology file\n");
        remove(METHODOLOGY_FILE_NAME);
        return true;
    }
    
    // Clear existing methods to avoid conflicts, using ar_method_destroy to free method objects
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
    
    // For each method type
    for (int i = 0; i < method_count; i++) {
        char name[MAX_METHOD_NAME_LENGTH];
        int version_count;
        
        // Read method name and version count
        if (fscanf(mut_fp, "%63s %d", name, &version_count) != 2 || version_count <= 0 || 
            version_count > MAX_VERSIONS_PER_METHOD) {
            printf("Error: Malformed method entry in %s\n", METHODOLOGY_FILE_NAME);
            fclose(mut_fp);
            // Delete the corrupted file and start fresh
            printf("Deleting corrupted methodology file\n");
            remove(METHODOLOGY_FILE_NAME);
            return true;
        }
        
        // Add this method to the methods array
        int method_idx = method_name_count++;
        method_counts[method_idx] = 0; // Reset count for this method
        
        // For each version
        for (int j = 0; j < version_count; j++) {
            if (method_counts[method_idx] >= MAX_VERSIONS_PER_METHOD) {
                printf("Error: Maximum versions reached for method %s\n", name);
                fclose(mut_fp);
                return false;
            }
            
            char version[32]; // Buffer for version string
            
            // Read method metadata (just version string now)
            if (fscanf(mut_fp, "%31s", version) != 1) {
                printf("Error: Malformed version entry in %s\n", METHODOLOGY_FILE_NAME);
                fclose(mut_fp);
                // Delete the corrupted file and start fresh
                printf("Deleting corrupted methodology file\n");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Skip newline
            getc(mut_fp);
            
            // Allocate space for instructions
            char instructions[MAX_INSTRUCTIONS_LENGTH];
            
            // Read the instructions
            if (fgets(instructions, MAX_INSTRUCTIONS_LENGTH, mut_fp) == NULL) {
                printf("Error: Could not read instructions for method %s version %s\n", 
                       name, version);
                fclose(mut_fp);
                // Delete the corrupted file and start fresh
                printf("Deleting corrupted methodology file\n");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Remove trailing newline if present
            size_t len = strlen(instructions);
            if (len > 0 && instructions[len-1] == '\n') {
                instructions[len-1] = '\0';
            }
            
            // Create a new method with the version from the file
            method_t *own_method = ar_method_create(name, instructions, version);
            
            if (own_method) {
                // Store the method directly in the methods array
                ar_methodology_set_method_storage(method_idx, method_counts[method_idx]++, own_method);
                own_method = NULL; // Mark as transferred
            }
        }
    }
    
    fclose(mut_fp);
    return true;
}
