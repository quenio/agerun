#include "ar_method.h"
#include "ar_heap.h"
#include "ar_assert.h" /* Include the assertion utilities */

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
const char* ar__method__get_name(const method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->name;
}

const char* ar__method__get_version(const method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->version;
}

// Removed ar_method_is_backward_compatible and ar_method_is_persistent implementations

const char* ar__method__get_instructions(const method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->instructions;
}

void ar__method__destroy(method_t *own_method) {
    if (own_method) {
        AR__HEAP__FREE(own_method);
    }
}

/**
 * Creates a new method with the given parameters
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return Newly created method object, or NULL on failure
 * @note Ownership: Returns an owned object that the caller must destroy with ar__method__destroy.
 *       The method copies the name, instructions, and version. The original strings remain owned by the caller.
 */
method_t* ar__method__create(const char *ref_name, const char *ref_instructions, 
                         const char *ref_version) {
    if (!ref_name || !ref_instructions || !ref_version) {
        return NULL;
    }
    
    // Allocate memory for the new method
    method_t *mut_method = AR__HEAP__MALLOC(sizeof(method_t), "Method structure");
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

