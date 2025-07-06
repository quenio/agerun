#include "ar_method.h"
#include "ar_method_ast.h"
#include "ar_method_parser.h"
#include "ar_heap.h"
#include "ar_log.h"
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
    ar_method_ast_t *own_ast;  // Parsed AST (owned by method)
};

/* Accessor functions implementation */
const char* ar_method__get_name(const ar_method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->name;
}

const char* ar_method__get_version(const ar_method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->version;
}

// Removed ar_method_is_backward_compatible and ar_method_is_persistent implementations

const char* ar_method__get_instructions(const ar_method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->instructions;
}

void ar_method__destroy(ar_method_t *own_method) {
    if (own_method) {
        // Destroy the AST if it exists
        if (own_method->own_ast) {
            ar_method_ast__destroy(own_method->own_ast);
        }
        AR__HEAP__FREE(own_method);
    }
}

/**
 * Creates a new method with the given parameters
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return Newly created method object, or NULL on failure
 * @note Ownership: Returns an owned object that the caller must destroy with ar_method__destroy.
 *       The method copies the name, instructions, and version. The original strings remain owned by the caller.
 */
ar_method_t* ar_method__create(const char *ref_name, const char *ref_instructions, 
                         const char *ref_version) {
    return ar_method__create_with_log(ref_name, ref_instructions, ref_version, NULL);
}

/**
 * Creates a new method with the given parameters and log support
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @param ref_log Optional log instance for error reporting (borrowed reference, may be NULL)
 * @return Newly created method object, or NULL on failure
 * @note Ownership: Returns an owned object that the caller must destroy with ar_method__destroy.
 *       The method copies the name, instructions, and version. The original strings remain owned by the caller.
 */
ar_method_t* ar_method__create_with_log(const char *ref_name, const char *ref_instructions, 
                                    const char *ref_version, ar_log_t *ref_log) {
    if (!ref_name || !ref_instructions || !ref_version) {
        return NULL;
    }
    
    // Allocate memory for the new method
    ar_method_t *mut_method = AR__HEAP__MALLOC(sizeof(ar_method_t), "Method structure");
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
    
    // Parse the instructions into AST
    ar_method_parser_t *own_parser = ar_method_parser__create(ref_log);
    if (!own_parser) {
        AR__HEAP__FREE(mut_method);
        return NULL;
    }
    
    mut_method->own_ast = ar_method_parser__parse(own_parser, ref_instructions);
    if (!mut_method->own_ast) {
        // Parser will have logged error to ar_log if one was provided
        // Continue without AST for backward compatibility
    }
    
    ar_method_parser__destroy(own_parser);
    
    return mut_method;
}

const ar_method_ast_t* ar_method__get_ast(const ar_method_t *ref_method) {
    AR_ASSERT(ref_method != NULL, "Method pointer cannot be NULL");
    return ref_method->own_ast;
}

