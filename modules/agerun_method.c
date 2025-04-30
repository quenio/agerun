#include "agerun_method.h"
#include "agerun_methodology.h"
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
#define MAX_INSTRUCTIONS_LENGTH 16384
#define MAX_METHOD_NAME_LENGTH 64

version_t ar_method_create(const char *name, const char *instructions, 
                        version_t previous_version, bool backward_compatible, 
                        bool persist) {
    if (!name || !instructions) {
        return 0;
    }
    
    // Find or create method entry
    int method_idx = ar_methodology_find_method_idx(name);
    int *method_name_count = ar_methodology_get_method_name_count();
    int *method_counts = ar_methodology_get_method_counts();
    
    if (method_idx < 0) {
        if (*method_name_count >= 256) { // MAX_METHODS
            printf("Error: Maximum number of method types reached\n");
            return 0;
        }
        
        method_idx = (*method_name_count)++;
        method_t *method = ar_methodology_get_method_storage(method_idx, 0);
        strncpy(method->name, name, MAX_METHOD_NAME_LENGTH - 1);
        method->name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
    }
    
    // Check if we've reached max versions for this method
    if (method_counts[method_idx] >= 64) { // MAX_VERSIONS_PER_METHOD
        printf("Error: Maximum number of versions reached for method %s\n", name);
        return 0;
    }
    
    // Create new version
    int version_idx = method_counts[method_idx]++;
    version_t new_version = previous_version + 1;
    
    // Make sure the version is unique
    for (int i = 0; i < version_idx; i++) {
        method_t *method = ar_methodology_get_method_storage(method_idx, i);
        if (method->version == new_version) {
            new_version = method->version + 1;
        }
    }
    
    // Initialize the new method version
    method_t *new_method = ar_methodology_get_method_storage(method_idx, version_idx);
    strncpy(new_method->name, name, MAX_METHOD_NAME_LENGTH - 1);
    new_method->name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
    new_method->version = new_version;
    new_method->previous_version = previous_version;
    new_method->backward_compatible = backward_compatible;
    new_method->persist = persist;
    strncpy(new_method->instructions, instructions, MAX_INSTRUCTIONS_LENGTH - 1);
    new_method->instructions[MAX_INSTRUCTIONS_LENGTH - 1] = '\0';
    
    printf("Created method %s version %d\n", name, new_version);
    
    return new_version;
}

bool ar_method_run(agent_t *mut_agent, data_t *mut_message, const char *ref_instructions) {
    if (!mut_agent || !ref_instructions) {
        return false;
    }
    
    // Make a copy of the instructions for tokenization
    char *own_instructions_copy = strdup(ref_instructions);
    if (!own_instructions_copy) {
        return false;
    }
    
    // Split instructions by newlines
    char *mut_instruction = strtok(own_instructions_copy, "\n");
    bool result = true;
    
    while (mut_instruction != NULL) {
        mut_instruction = ar_string_trim(mut_instruction);
        
        // Skip empty lines and comments
        if (strlen(mut_instruction) > 0 && mut_instruction[0] != '#') {
            if (!ar_instruction_run(mut_agent, mut_message, mut_instruction)) {
                result = false;
                break;
            }
        }
        
        mut_instruction = strtok(NULL, "\n");
    }
    
    free(own_instructions_copy);
    own_instructions_copy = NULL; // Mark as freed
    return result;
}
