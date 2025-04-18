#include "agerun_methodology.h"
#include "agerun_method.h"
#include "agerun_string.h"

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
static method_t methods[MAX_METHODS][MAX_VERSIONS_PER_METHOD];
static int method_counts[MAX_METHODS];
static int method_name_count = 0;

/* Forward Declarations */
static int find_method_idx(const char *name);
static method_t* find_latest_method(const char *name);
static method_t* find_method(const char *name, version_t version);

/* Method Search Functions */
static int find_method_idx(const char *name) {
    for (int i = 0; i < method_name_count; i++) {
        if (strcmp(methods[i][0].name, name) == 0) {
            return i;
        }
    }
    
    return -1;
}

static method_t* find_latest_method(const char *name) {
    int method_idx = find_method_idx(name);
    if (method_idx < 0 || method_counts[method_idx] == 0) {
        return NULL;
    }
    
    // Find the most recent version
    version_t latest_version = 0;
    int latest_idx = -1;
    
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i].version > latest_version) {
            latest_version = methods[method_idx][i].version;
            latest_idx = i;
        }
    }
    
    if (latest_idx >= 0) {
        return &methods[method_idx][latest_idx];
    }
    
    return NULL;
}

static method_t* find_method(const char *name, version_t version) {
    int method_idx = find_method_idx(name);
    if (method_idx < 0) {
        return NULL;
    }
    
    // Case 1: Exact version match
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i].version == version) {
            return &methods[method_idx][i];
        }
    }
    
    // Case 2: Find compatible version
    version_t latest_compatible = 0;
    int latest_idx = -1;
    
    for (int i = 0; i < method_counts[method_idx]; i++) {
        if (methods[method_idx][i].backward_compatible && 
            methods[method_idx][i].version > version && 
            methods[method_idx][i].version > latest_compatible) {
            latest_compatible = methods[method_idx][i].version;
            latest_idx = i;
        }
    }
    
    if (latest_idx >= 0) {
        return &methods[method_idx][latest_idx];
    }
    
    return NULL; // No compatible version found
}

// Interface functions for agerun_method.c
int ar_methodology_find_method_idx(const char *name) {
    return find_method_idx(name);
}

method_t* ar_methodology_get_method_storage(int method_idx, int version_idx) {
    return &methods[method_idx][version_idx];
}

int* ar_methodology_get_method_counts(void) {
    return method_counts;
}

int* ar_methodology_get_method_name_count(void) {
    return &method_name_count;
}

// Main method access function
method_t* ar_methodology_get_method(const char *name, version_t version) {
    if (version == 0) {
        // Use latest version
        return find_latest_method(name);
    } else {
        // Use specific version
        return find_method(name, version);
    }
}

bool ar_methodology_save_methods(void) {
    // Simple placeholder implementation
    FILE *fp = fopen(METHODOLOGY_FILE_NAME, "w");
    if (!fp) {
        printf("Error: Could not open %s for writing\n", METHODOLOGY_FILE_NAME);
        return false;
    }
    
    // Write the number of method types
    fprintf(fp, "%d\n", method_name_count);
    
    // For each method type
    for (int i = 0; i < method_name_count; i++) {
        // Write the method name and number of versions
        fprintf(fp, "%s %d\n", methods[i][0].name, method_counts[i]);
        
        // For each version
        for (int j = 0; j < method_counts[i]; j++) {
            method_t *method = &methods[i][j];
            
            // Write method metadata
            fprintf(fp, "%d %d %d\n", 
                    method->version, 
                    method->backward_compatible ? 1 : 0,
                    method->persist ? 1 : 0);
            
            // Write instructions (base64 encoded or other suitable format)
            fprintf(fp, "%s\n", method->instructions);
        }
    }
    
    fclose(fp);
    return true;
}

bool ar_methodology_load_methods(void) {
    FILE *fp = fopen(METHODOLOGY_FILE_NAME, "r");
    if (!fp) {
        // Not an error, might be first run
        return true;
    }
    
    // Read the number of method types
    int method_count = 0;
    if (fscanf(fp, "%d", &method_count) != 1 || method_count <= 0 || method_count > MAX_METHODS) {
        printf("Error: Invalid method count in %s\n", METHODOLOGY_FILE_NAME);
        fclose(fp);
        // Delete the corrupted file and start fresh
        printf("Deleting corrupted methodology file\n");
        remove(METHODOLOGY_FILE_NAME);
        return true;
    }
    
    // Clear existing methods to avoid conflicts
    for (int i = 0; i < method_name_count; i++) {
        method_counts[i] = 0;
    }
    method_name_count = 0;
    
    // For each method type
    for (int i = 0; i < method_count; i++) {
        char name[MAX_METHOD_NAME_LENGTH];
        int version_count;
        
        // Read method name and version count
        if (fscanf(fp, "%63s %d", name, &version_count) != 2 || version_count <= 0 || 
            version_count > MAX_VERSIONS_PER_METHOD) {
            printf("Error: Malformed method entry in %s\n", METHODOLOGY_FILE_NAME);
            fclose(fp);
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
                fclose(fp);
                return false;
            }
            
            version_t version;
            int backward_compatible, persist;
            
            // Read method metadata
            if (fscanf(fp, "%d %d %d", &version, &backward_compatible, &persist) != 3) {
                printf("Error: Malformed version entry in %s\n", METHODOLOGY_FILE_NAME);
                fclose(fp);
                // Delete the corrupted file and start fresh
                printf("Deleting corrupted methodology file\n");
                remove(METHODOLOGY_FILE_NAME);
                return true;
            }
            
            // Skip newline
            getc(fp);
            
            // Allocate space for instructions
            char instructions[MAX_INSTRUCTIONS_LENGTH];
            
            // Read the instructions
            if (fgets(instructions, MAX_INSTRUCTIONS_LENGTH, fp) == NULL) {
                printf("Error: Could not read instructions for method %s version %d\n", 
                       name, version);
                fclose(fp);
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
            
            // Register the method
            method_t *method = &methods[method_idx][method_counts[method_idx]++];
            strncpy(method->name, name, MAX_METHOD_NAME_LENGTH - 1);
            method->name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
            method->version = version;
            method->backward_compatible = backward_compatible != 0;
            method->persist = persist != 0;
            strncpy(method->instructions, instructions, MAX_INSTRUCTIONS_LENGTH - 1);
            method->instructions[MAX_INSTRUCTIONS_LENGTH - 1] = '\0';
        }
    }
    
    fclose(fp);
    return true;
}
