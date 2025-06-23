/**
 * @file agerun_condition_instruction_evaluator.c
 * @brief Implementation of the condition instruction evaluator module
 */

#include "agerun_condition_instruction_evaluator.h"
#include "agerun_heap.h"
#include "agerun_expression_ast.h"
#include "agerun_list.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>


/**
 * Internal structure for condition instruction evaluator
 */
struct ar_condition_instruction_evaluator_s {
    ar_expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    data_t *mut_memory;                          /* Memory map (mutable reference) */
};

/* Constants */
static const char* MEMORY_PREFIX = "memory.";
static const size_t MEMORY_PREFIX_LEN = 7;

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
    
    switch (ar__data__get_type(ref_value)) {
        case DATA_INTEGER:
            return ar__data__create_integer(ar__data__get_integer(ref_value));
        case DATA_DOUBLE:
            return ar__data__create_double(ar__data__get_double(ref_value));
        case DATA_STRING:
            return ar__data__create_string(ar__data__get_string(ref_value));
        case DATA_MAP:
            {
                // Create a new map and copy all key-value pairs
                data_t *new_map = ar__data__create_map();
                if (!new_map) return NULL;
                
                // Get all keys from the original map
                data_t *keys = ar__data__get_map_keys(ref_value);
                if (!keys) {
                    ar__data__destroy(new_map);
                    return NULL;
                }
                
                // Copy each key-value pair
                size_t count = ar__data__list_count(keys);
                for (size_t i = 0; i < count; i++) {
                    // Get the key
                    data_t *key_data = ar__data__list_first(keys);
                    if (!key_data) break;
                    
                    const char *key = ar__data__get_string(key_data);
                    if (!key) {
                        data_t *removed = ar__data__list_remove_first(keys);
                        ar__data__destroy(removed);
                        continue;
                    }
                    
                    // Get the value from the original map
                    data_t *orig_value = ar__data__get_map_data(ref_value, key);
                    if (orig_value) {
                        // Recursively copy the value
                        data_t *copy_value = _copy_data_value(orig_value);
                        if (copy_value) {
                            bool success = ar__data__set_map_data(new_map, key, copy_value);
                            if (!success) {
                                fprintf(stderr, "ERROR: Failed to set map data for key '%s'\n", key);
                                ar__data__destroy(copy_value);
                            }
                        }
                    }
                    
                    // Remove and destroy the processed key
                    data_t *removed_key = ar__data__list_remove_first(keys);
                    ar__data__destroy(removed_key);
                }
                
                // Clean up the keys list
                ar__data__destroy(keys);
                
                return new_map;
            }
        case DATA_LIST:
            // TODO: Implement deep copy for lists
            return ar__data__create_list();
        default:
            return NULL;
    }
}

/* Helper function to evaluate an expression AST node using the expression evaluator */
static data_t* _evaluate_expression_ast(ar_expression_evaluator_t *mut_expr_evaluator, const ar_expression_ast_t *ref_ast) {
    if (!ref_ast) {
        return NULL;
    }
    
    ar_expression_ast_type_t type = ar__expression_ast__get_type(ref_ast);
    
    switch (type) {
        case AR_EXPR__LITERAL_INT:
            return ar__expression_evaluator__evaluate_literal_int(mut_expr_evaluator, ref_ast);
            
        case AR_EXPR__LITERAL_DOUBLE:
            return ar__expression_evaluator__evaluate_literal_double(mut_expr_evaluator, ref_ast);
            
        case AR_EXPR__LITERAL_STRING:
            return ar__expression_evaluator__evaluate_literal_string(mut_expr_evaluator, ref_ast);
            
        case AR_EXPR__MEMORY_ACCESS:
            // Memory access returns a reference, we need to make a copy
            {
                data_t *ref_value = ar__expression_evaluator__evaluate_memory_access(mut_expr_evaluator, ref_ast);
                if (!ref_value) return NULL;
                
                // Create a deep copy of the value
                return _copy_data_value(ref_value);
            }
            
        case AR_EXPR__BINARY_OP:
            return ar__expression_evaluator__evaluate_binary_op(mut_expr_evaluator, ref_ast);
            
        default:
            return NULL;
    }
}

/**
 * Creates a new condition instruction evaluator
 */
ar_condition_instruction_evaluator_t* ar_condition_instruction_evaluator__create(
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
) {
    if (!ref_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_condition_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(
        sizeof(ar_condition_instruction_evaluator_t),
        "condition_instruction_evaluator"
    );
    if (!own_evaluator) {
        return NULL;
    }
    
    own_evaluator->ref_expr_evaluator = ref_expr_evaluator;
    own_evaluator->mut_memory = mut_memory;
    
    // Ownership transferred to caller
    return own_evaluator;
}

/**
 * Destroys a condition instruction evaluator
 */
void ar_condition_instruction_evaluator__destroy(
    ar_condition_instruction_evaluator_t *own_evaluator
) {
    if (!own_evaluator) {
        return;
    }
    
    // Note: We don't destroy the dependencies as they are borrowed references
    AR__HEAP__FREE(own_evaluator);
}

/**
 * Evaluates a condition (if) instruction using stored dependencies
 */
bool ar_condition_instruction_evaluator__evaluate(
    ar_condition_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Verify this is an if AST node
    if (ar__instruction_ast__get_type(ref_ast) != AR_INST__IF) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const list_t *ref_arg_asts = ar__instruction_ast__get_function_arg_asts(ref_ast);
    if (!ref_arg_asts) {
        return false;
    }
    
    // Verify we have exactly 3 arguments
    if (ar__list__count(ref_arg_asts) != 3) {
        return false;
    }
    
    // Get the argument ASTs array
    void **items = ar__list__items(ref_arg_asts);
    if (!items) {
        return false;
    }
    
    const ar_expression_ast_t *ref_condition_ast = (const ar_expression_ast_t*)items[0];
    const ar_expression_ast_t *ref_true_ast = (const ar_expression_ast_t*)items[1];
    const ar_expression_ast_t *ref_false_ast = (const ar_expression_ast_t*)items[2];
    
    if (!ref_condition_ast || !ref_true_ast || !ref_false_ast) {
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Evaluate condition expression
    data_t *own_condition_data = _evaluate_expression_ast(mut_evaluator->ref_expr_evaluator, ref_condition_ast);
    if (!own_condition_data) {
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Check condition value (0 is false, non-zero is true)
    bool condition_is_true = false;
    if (ar__data__get_type(own_condition_data) == DATA_INTEGER) {
        condition_is_true = (ar__data__get_integer(own_condition_data) != 0);
    }
    ar__data__destroy(own_condition_data);
    
    // Select which expression AST to evaluate based on condition
    const ar_expression_ast_t *ref_ast_to_eval = condition_is_true ? ref_true_ast : ref_false_ast;
    
    // Evaluate the selected expression AST
    data_t *own_result = _evaluate_expression_ast(mut_evaluator->ref_expr_evaluator, ref_ast_to_eval);
    
    // Clean up the items array as we're done with it
    AR__HEAP__FREE(items);
    
    if (!own_result) {
        return false;
    }
    
    // Handle result assignment if present
    const char *ref_result_path = ar__instruction_ast__get_function_result_path(ref_ast);
    if (ref_result_path) {
        // Get memory key path
        const char *key_path = _get_memory_key_path(ref_result_path);
        if (!key_path) {
            ar__data__destroy(own_result);
            return false;
        }
        
        // Store the result value (transfers ownership)
        bool store_success = ar__data__set_map_data(mut_evaluator->mut_memory, key_path, own_result);
        if (!store_success) {
            ar__data__destroy(own_result);
        }
        
        // For assignments, return true to indicate the instruction succeeded
        return true;
    } else {
        // No assignment, just return success (expression was evaluated for side effects)
        ar__data__destroy(own_result);
        return true;
    }
}