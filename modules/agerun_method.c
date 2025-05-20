#include "agerun_method.h"
#include "agerun_instruction.h"
#include "agerun_string.h"
#include "agerun_data.h"
#include "agerun_agent.h"
#include "agerun_map.h"
#include "agerun_heap.h"
#include "agerun_assert.h" /* Include the assertion utilities */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Constants */
#define MAX_INSTRUCTIONS_LENGTH 16384
#define MAX_METHOD_NAME_LENGTH 64

/* Method Definition (full structure) */
#define MAX_VERSION_LENGTH 16 // Enough for semver (e.g., "1.2.3")

struct method_s {
    char name[MAX_METHOD_NAME_LENGTH];
    char version[MAX_VERSION_LENGTH];
    char instructions[MAX_INSTRUCTIONS_LENGTH];
};

/* Accessor functions implementation */
const char* ar_method_get_name(const method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->name;
}

const char* ar_method_get_version(const method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->version;
}

// Removed ar_method_is_backward_compatible and ar_method_is_persistent implementations

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
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return Newly created method object, or NULL on failure
 * @note Ownership: Returns an owned object that the caller must destroy with ar_method_destroy.
 *       The method copies the name, instructions, and version. The original strings remain owned by the caller.
 */
method_t* ar_method_create(const char *ref_name, const char *ref_instructions, 
                         const char *ref_version) {
    if (!ref_name || !ref_instructions || !ref_version) {
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
    
    strncpy(mut_method->version, ref_version, MAX_VERSION_LENGTH - 1);
    mut_method->version[MAX_VERSION_LENGTH - 1] = '\0';
    
    strncpy(mut_method->instructions, ref_instructions, MAX_INSTRUCTIONS_LENGTH - 1);
    mut_method->instructions[MAX_INSTRUCTIONS_LENGTH - 1] = '\0';
    
    return mut_method;
}

bool ar_method_run(agent_t *mut_agent, const data_t *ref_message, const char *ref_instructions) {
    if (!mut_agent || !ref_instructions) {
        return false;
    }
    
    // Create an instruction context
    instruction_context_t *own_ctx = ar_instruction_create_context(
        mut_agent->own_memory,
        mut_agent->ref_context,
        ref_message
    );
    
    if (!own_ctx) {
        return false;
    }
    
    // Make a copy of the instructions for tokenization
    char *own_instructions_copy = strdup(ref_instructions);
    if (!own_instructions_copy) {
        ar_instruction_destroy_context(own_ctx);
        return false;
    }
    
    // Split instructions by newlines
    char *mut_instruction = strtok(own_instructions_copy, "\n");
    bool result = true;
    
    while (mut_instruction != NULL) {
        mut_instruction = ar_string_trim(mut_instruction);
        
        // Skip empty lines and comments
        if (strlen(mut_instruction) > 0 && mut_instruction[0] != '#') {
            if (!ar_instruction_run(own_ctx, mut_instruction)) {
                result = false;
                break;
            }
        }
        
        mut_instruction = strtok(NULL, "\n");
    }
    
    // Clean up
    ar_instruction_destroy_context(own_ctx);
    free(own_instructions_copy);
    own_instructions_copy = NULL; // Mark as freed
    
    return result;
}
