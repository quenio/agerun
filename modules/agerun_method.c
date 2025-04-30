#include "agerun_method.h"
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

void ar_method_destroy(method_t *own_method) {
    if (own_method) {
        free(own_method);
    }
}

/**
 * Creates a new method with the given parameters
 * This function creates and returns an allocated method object that the caller takes ownership of.
 */
method_t* ar_method_create_object(const char *ref_name, const char *ref_instructions, 
                         version_t version, version_t previous_version, 
                         bool backward_compatible, bool persist) {
    if (!ref_name || !ref_instructions) {
        return NULL;
    }
    
    // Allocate memory for the new method
    method_t *mut_method = malloc(sizeof(method_t));
    if (!mut_method) {
        printf("Error: Failed to allocate memory for method\n");
        return NULL;
    }
    
    // Initialize the method fields
    strncpy(mut_method->name, ref_name, MAX_METHOD_NAME_LENGTH - 1);
    mut_method->name[MAX_METHOD_NAME_LENGTH - 1] = '\0';
    
    mut_method->version = version;
    mut_method->previous_version = previous_version;
    mut_method->backward_compatible = backward_compatible;
    mut_method->persist = persist;
    strncpy(mut_method->instructions, ref_instructions, MAX_INSTRUCTIONS_LENGTH - 1);
    mut_method->instructions[MAX_INSTRUCTIONS_LENGTH - 1] = '\0';
    
    return mut_method;
}

version_t ar_method_create(const char *ref_name, const char *ref_instructions, 
                        version_t previous_version, bool backward_compatible, 
                        bool persist) {
    if (!ref_name || !ref_instructions) {
        return 0;
    }
    
    version_t version = previous_version + 1;
    
    // Create a new method object with the next version number
    method_t *mut_method = ar_method_create_object(ref_name, ref_instructions, 
                                          version, previous_version, 
                                          backward_compatible, persist);
    if (!mut_method) {
        return 0;
    }
    
    // Let the methodology module handle this - it will track, store, and own the method
    extern void ar_methodology_register_method(method_t *own_method);
    ar_methodology_register_method(mut_method);
    
    // Return the version number
    return version;
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
