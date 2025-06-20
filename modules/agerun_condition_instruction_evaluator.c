/**
 * @file agerun_condition_instruction_evaluator.c
 * @brief Implementation of the condition instruction evaluator module
 */

#include "agerun_condition_instruction_evaluator.h"
#include "agerun_heap.h"
#include "agerun_expression_parser.h"
#include "agerun_expression_ast.h"
#include "agerun_list.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

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

/* Helper function to extract function arguments from AST node */
static void** _extract_function_args(const instruction_ast_t *ref_ast, size_t expected_count, list_t **out_args_list) {
    // Get the argument list from the AST
    list_t *own_args = ar__instruction_ast__get_function_args(ref_ast);
    if (!own_args) {
        return NULL;
    }
    
    // Check argument count
    size_t count = ar__list__count(own_args);
    if (count != expected_count) {
        ar__list__destroy(own_args);
        return NULL;
    }
    
    // Get items array (borrowed reference)
    void **items = ar__list__items(own_args);
    if (!items) {
        ar__list__destroy(own_args);
        return NULL;
    }
    
    // Return the items array and pass ownership of the list to caller
    *out_args_list = own_args;
    return items;
}

/* Helper function to clean up function arguments */
static void _cleanup_function_args(void **own_items, list_t *own_args) {
    if (own_items) {
        AR__HEAP__FREE(own_items);
    }
    if (own_args) {
        ar__list__destroy(own_args);
    }
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
static data_t* _evaluate_expression_ast(expression_evaluator_t *mut_expr_evaluator, expression_ast_t *ref_ast) {
    if (!ref_ast) {
        return NULL;
    }
    
    expression_ast_type_t type = ar__expression_ast__get_type(ref_ast);
    
    switch (type) {
        case EXPR_AST_LITERAL_INT:
            return ar__expression_evaluator__evaluate_literal_int(mut_expr_evaluator, ref_ast);
            
        case EXPR_AST_LITERAL_DOUBLE:
            return ar__expression_evaluator__evaluate_literal_double(mut_expr_evaluator, ref_ast);
            
        case EXPR_AST_LITERAL_STRING:
            return ar__expression_evaluator__evaluate_literal_string(mut_expr_evaluator, ref_ast);
            
        case EXPR_AST_MEMORY_ACCESS:
            // Memory access returns a reference, we need to make a copy
            {
                data_t *ref_value = ar__expression_evaluator__evaluate_memory_access(mut_expr_evaluator, ref_ast);
                if (!ref_value) return NULL;
                
                // Create a deep copy of the value
                return _copy_data_value(ref_value);
            }
            
        case EXPR_AST_BINARY_OP:
            return ar__expression_evaluator__evaluate_binary_op(mut_expr_evaluator, ref_ast);
            
        default:
            return NULL;
    }
}

/**
 * Evaluates a condition (if) instruction AST node
 */
bool ar_condition_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
) {
    if (!mut_expr_evaluator || !mut_memory || !ref_ast) {
        return false;
    }
    
    // Verify this is an if AST node
    if (ar__instruction_ast__get_type(ref_ast) != INST_AST_IF) {
        return false;
    }
    
    // Get function arguments
    list_t *own_args = NULL;
    void **items = _extract_function_args(ref_ast, 3, &own_args);
    if (!items) {
        return false;
    }
    
    const char *ref_condition_expr = (const char*)items[0];
    const char *ref_true_expr = (const char*)items[1];
    const char *ref_false_expr = (const char*)items[2];
    
    if (!ref_condition_expr || !ref_true_expr || !ref_false_expr) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    // Parse and evaluate condition expression
    expression_parser_t *parser = ar__expression_parser__create(ref_condition_expr);
    if (!parser) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    expression_ast_t *condition_ast = ar__expression_parser__parse_expression(parser);
    ar__expression_parser__destroy(parser);
    
    if (!condition_ast) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    data_t *own_condition_data = _evaluate_expression_ast(mut_expr_evaluator, condition_ast);
    ar__expression_ast__destroy(condition_ast);
    
    if (!own_condition_data) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    // Check condition value (0 is false, non-zero is true)
    bool condition_is_true = false;
    if (ar__data__get_type(own_condition_data) == DATA_INTEGER) {
        condition_is_true = (ar__data__get_integer(own_condition_data) != 0);
    }
    ar__data__destroy(own_condition_data);
    
    // Select which expression to evaluate based on condition
    const char *ref_expr_to_eval = condition_is_true ? ref_true_expr : ref_false_expr;
    
    // Parse and evaluate the selected expression
    parser = ar__expression_parser__create(ref_expr_to_eval);
    if (!parser) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    expression_ast_t *value_ast = ar__expression_parser__parse_expression(parser);
    ar__expression_parser__destroy(parser);
    
    if (!value_ast) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    data_t *own_result = _evaluate_expression_ast(mut_expr_evaluator, value_ast);
    ar__expression_ast__destroy(value_ast);
    
    if (!own_result) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    // Clean up items array and args list
    _cleanup_function_args(items, own_args);
    
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
        bool store_success = ar__data__set_map_data(mut_memory, key_path, own_result);
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