#include "agerun_method.h"
#include "agerun_instruction.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_agent.h"
#include "agerun_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

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

version_t ar_method_create(const char *name, const char *instructions, 
                        version_t previous_version, bool backward_compatible, 
                        bool persist) {
    if (!name || !instructions) {
        return 0;
    }
    
    // Find or create method entry
    int method_idx = find_method_idx(name);
    if (method_idx < 0) {
        if (method_name_count >= MAX_METHODS) {
            printf("Error: Maximum number of method types reached\n");
            return 0;
        }
        
        method_idx = method_name_count++;
        strncpy(methods[method_idx][0].name, name, MAX_METHOD_NAME_LENGTH - 1);
        methods[method_idx][0].name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
    }
    
    // Check if we've reached max versions for this method
    if (method_counts[method_idx] >= MAX_VERSIONS_PER_METHOD) {
        printf("Error: Maximum number of versions reached for method %s\n", name);
        return 0;
    }
    
    // Create new version
    int version_idx = method_counts[method_idx]++;
    version_t new_version = previous_version + 1;
    
    // Make sure the version is unique
    for (int i = 0; i < version_idx; i++) {
        if (methods[method_idx][i].version == new_version) {
            new_version = methods[method_idx][i].version + 1;
        }
    }
    
    // Initialize the new method version
    strncpy(methods[method_idx][version_idx].name, name, MAX_METHOD_NAME_LENGTH - 1);
    methods[method_idx][version_idx].name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
    methods[method_idx][version_idx].version = new_version;
    methods[method_idx][version_idx].previous_version = previous_version;
    methods[method_idx][version_idx].backward_compatible = backward_compatible;
    methods[method_idx][version_idx].persist = persist;
    strncpy(methods[method_idx][version_idx].instructions, instructions, MAX_INSTRUCTIONS_LENGTH - 1);
    methods[method_idx][version_idx].instructions[MAX_INSTRUCTIONS_LENGTH - 1] = '\0';
    
    printf("Created method %s version %d\n", name, new_version);
    
    return new_version;
}

// Main interpretation function for agent methods
method_t* ar_method_get(const char *name, version_t version) {
    if (version == 0) {
        // Use latest version
        return find_latest_method(name);
    } else {
        // Use specific version
        return find_method(name, version);
    }
}

bool ar_method_run(agent_t *agent, const char *message, const char *instructions) {
    (void)agent; // Avoid unused parameter warning
    (void)message; // Avoid unused parameter warning
    
    // Make a copy of the instructions for tokenization
    char *instructions_copy = strdup(instructions);
    if (!instructions_copy) {
        return false;
    }
    
    // Split instructions by newlines
    char *instruction = strtok(instructions_copy, "\n");
    bool result = true;
    
    while (instruction != NULL) {
        instruction = ar_trim(instruction);
        
        // Skip empty lines and comments
        if (strlen(instruction) > 0 && instruction[0] != '#') {
            if (!ar_instruction_run(agent, message, instruction)) {
                result = false;
                break;
            }
        }
        
        instruction = strtok(NULL, "\n");
    }
    
    free(instructions_copy);
    return result;
}
