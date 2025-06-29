/**
 * @file agerun_assignment_instruction_evaluator.c
 * @brief Implementation of the assignment instruction evaluator module
 */

#include "ar_assignment_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_expression_ast.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>


/* Internal structure for the assignment instruction evaluator */
struct ar_assignment_instruction_evaluator_s {
    ar_expression_evaluator_t *ref_expr_evaluator;  /* Borrowed reference to expression evaluator */
    data_t *mut_memory;                          /* Mutable reference to memory map */
};

/* Constants */
static const char* MEMORY_PREFIX = "memory.";
static const size_t MEMORY_PREFIX_LEN = 7;

ar_assignment_instruction_evaluator_t* ar_assignment_instruction_evaluator__create(
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
) {
    if (!ref_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_assignment_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(sizeof(ar_assignment_instruction_evaluator_t), "assignment_instruction_evaluator");
    if (!own_evaluator) {
        return NULL;
    }
    
    own_evaluator->ref_expr_evaluator = ref_expr_evaluator;
    own_evaluator->mut_memory = mut_memory;
    
    // Ownership transferred to caller
    return own_evaluator;
}

void ar_assignment_instruction_evaluator__destroy(
    ar_assignment_instruction_evaluator_t *own_evaluator
) {
    if (!own_evaluator) {
        return;
    }
    
    // Just free the struct, we don't own the expression evaluator or memory
    AR__HEAP__FREE(own_evaluator);
}

/* Helper function to check if a path starts with "memory." and return the key path */
static const char* _get_memory_key_path(const char *ref_path) {
    if (!ref_path) {
        return NULL;
    }
    
    if (strncmp(ref_path, MEMORY_PREFIX, MEMORY_PREFIX_LEN) != 0) {
        return NULL;
    }
    
    return ref_path + MEMORY_PREFIX_LEN;
}

/* Helper function to create a deep copy of data value */
static data_t* _copy_data_value(const data_t *ref_value) {
    if (!ref_value) {
        return NULL;
    }
    
    switch (ar_data__get_type(ref_value)) {
        case DATA_INTEGER:
            return ar_data__create_integer(ar_data__get_integer(ref_value));
        case DATA_DOUBLE:
            return ar_data__create_double(ar_data__get_double(ref_value));
        case DATA_STRING:
            return ar_data__create_string(ar_data__get_string(ref_value));
        case DATA_MAP:
            {
                // Create a new map and copy all key-value pairs
                data_t *new_map = ar_data__create_map();
                if (!new_map) return NULL;
                
                // Get all keys from the original map
                data_t *keys = ar_data__get_map_keys(ref_value);
                if (!keys) {
                    ar_data__destroy(new_map);
                    return NULL;
                }
                
                // Copy each key-value pair
                size_t count = ar_data__list_count(keys);
                for (size_t i = 0; i < count; i++) {
                    // Get the key
                    data_t *key_data = ar_data__list_first(keys);
                    if (!key_data) break;
                    
                    const char *key = ar_data__get_string(key_data);
                    if (!key) {
                        data_t *removed = ar_data__list_remove_first(keys);
                        ar_data__destroy(removed);
                        continue;
                    }
                    
                    // Get the value from the original map
                    data_t *orig_value = ar_data__get_map_data(ref_value, key);
                    if (orig_value) {
                        // Recursively copy the value
                        data_t *copy_value = _copy_data_value(orig_value);
                        if (copy_value) {
                            bool success = ar_data__set_map_data(new_map, key, copy_value);
                            if (!success) {
                                fprintf(stderr, "ERROR: Failed to set map data for key '%s'\n", key);
                                ar_data__destroy(copy_value);
                            }
                        }
                    }
                    
                    // Remove and destroy the processed key
                    data_t *removed_key = ar_data__list_remove_first(keys);
                    ar_data__destroy(removed_key);
                }
                
                // Clean up the keys list
                ar_data__destroy(keys);
                
                return new_map;
            }
        case DATA_LIST:
            // TODO: Implement deep copy for lists
            return ar_data__create_list();
        default:
            return NULL;
    }
}


/**
 * Evaluates an assignment instruction AST node
 */
bool ar_assignment_instruction_evaluator__evaluate(
    ar_assignment_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Verify this is an assignment AST node
    if (ar_instruction_ast__get_type(ref_ast) != AR_INST__ASSIGNMENT) {
        return false;
    }
    
    // Get assignment details
    const char *ref_path = ar_instruction_ast__get_assignment_path(ref_ast);
    if (!ref_path) {
        return false;
    }
    
    // Get memory key path
    const char *key_path = _get_memory_key_path(ref_path);
    if (!key_path) {
        return false;
    }
    
    // Get the pre-parsed expression AST
    const ar_expression_ast_t *ref_expr_ast = ar_instruction_ast__get_assignment_expression_ast(ref_ast);
    if (!ref_expr_ast) {
        return false;
    }
    
    // Evaluate the expression AST
    data_t *result = ar_expression_evaluator__evaluate(mut_evaluator->ref_expr_evaluator, ref_expr_ast);
    
    if (!result) {
        return false;
    }
    
    // Check if we need to make a copy (if result is owned by memory/context)
    data_t *own_value;
    if (ar_data__hold_ownership(result, mut_evaluator)) {
        // We can claim ownership - it's an unowned value (literal or operation result)
        ar_data__transfer_ownership(result, mut_evaluator);  // Transfer to NULL
        own_value = result;
    } else {
        // It's owned by someone else (memory access) - we need to make a copy
        own_value = _copy_data_value(result);
        if (!own_value) {
            return false;
        }
    }
    
    // Store the value in memory (transfers ownership)
    bool success = ar_data__set_map_data(mut_evaluator->mut_memory, key_path, own_value);
    if (!success) {
        ar_data__destroy(own_value);
    }
    
    return success;
}

