#include "agerun_method.h"
#include "agerun_methodology.h"
#include "agerun_instruction.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_agent.h"
#include "agerun_map.h"
#include "agerun_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Constants */
#define MAX_INSTRUCTIONS_LENGTH 16384
#define MAX_METHOD_NAME_LENGTH 64

/* Method Definition (full structure) */
struct method_s {
    char name[MAX_METHOD_NAME_LENGTH];
    version_t version;
    version_t previous_version;
    bool backward_compatible;
    bool persist;
    char instructions[MAX_INSTRUCTIONS_LENGTH];
};

/* Accessor functions implementation */
const char* ar_method_get_name(const method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->name;
}

version_t ar_method_get_version(const method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->version;
}

version_t ar_method_get_previous_version(const method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->previous_version;
}

bool ar_method_is_backward_compatible(const method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->backward_compatible;
}

bool ar_method_is_persistent(const method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->persist;
}

const char* ar_method_get_instructions(const method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->instructions;
}

version_t ar_method_create(const char *ref_name, const char *ref_instructions, 
                        version_t previous_version, bool backward_compatible, 
                        bool persist) {
    if (!ref_name || !ref_instructions) {
        return 0;
    }
    
    // Find or create method entry
    int method_idx = ar_methodology_find_method_idx(ref_name);
    int *mut_method_name_count = ar_methodology_get_method_name_count();
    int *mut_method_counts = ar_methodology_get_method_counts();
    
    if (method_idx < 0) {
        if (*mut_method_name_count >= 256) { // MAX_METHODS
            printf("Error: Maximum number of method types reached\n");
            return 0;
        }
        
        method_idx = (*mut_method_name_count)++;
        method_t *mut_method = ar_methodology_get_method_storage(method_idx, 0);
        strncpy(mut_method->name, ref_name, MAX_METHOD_NAME_LENGTH - 1);
        mut_method->name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
    }
    
    // Check if we've reached max versions for this method
    if (mut_method_counts[method_idx] >= 64) { // MAX_VERSIONS_PER_METHOD
        printf("Error: Maximum number of versions reached for method %s\n", ref_name);
        return 0;
    }
    
    // Create new version
    int version_idx = mut_method_counts[method_idx]++;
    version_t new_version = previous_version + 1;
    
    // Make sure the version is unique
    for (int i = 0; i < version_idx; i++) {
        method_t *ref_method = ar_methodology_get_method_storage(method_idx, i);
        if (ref_method->version == new_version) {
            new_version = ref_method->version + 1;
        }
    }
    
    // Initialize the new method version
    method_t *mut_new_method = ar_methodology_get_method_storage(method_idx, version_idx);
    strncpy(mut_new_method->name, ref_name, MAX_METHOD_NAME_LENGTH - 1);
    mut_new_method->name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
    mut_new_method->version = new_version;
    mut_new_method->previous_version = previous_version;
    mut_new_method->backward_compatible = backward_compatible;
    mut_new_method->persist = persist;
    strncpy(mut_new_method->instructions, ref_instructions, MAX_INSTRUCTIONS_LENGTH - 1);
    mut_new_method->instructions[MAX_INSTRUCTIONS_LENGTH - 1] = '\0';
    
    printf("Created method %s version %d\n", ref_name, new_version);
    
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
