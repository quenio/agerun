/**
 * @file agerun_destroy_agent_instruction_evaluator.c
 * @brief Implementation of the destroy agent instruction evaluator module
 */

#include "agerun_destroy_agent_instruction_evaluator.h"
#include "agerun_heap.h"
#include "agerun_expression_parser.h"
#include "agerun_expression_ast.h"
#include "agerun_agency.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>


/* Constants */
static const char* MEMORY_PREFIX = "memory.";
static const size_t MEMORY_PREFIX_LEN = 7;

/* Opaque struct definition */
struct ar_destroy_agent_instruction_evaluator_s {
    expression_evaluator_t *mut_expr_evaluator;
    data_t *mut_memory;
};

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
static data_t* _evaluate_expression_ast(
    expression_evaluator_t *mut_expr_evaluator,
    expression_ast_t *ref_ast
) {
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

/* Helper function to parse and evaluate an expression string */
static data_t* _parse_and_evaluate_expression(
    expression_evaluator_t *mut_expr_evaluator,
    const char *ref_expr
) {
    if (!ref_expr) {
        return NULL;
    }
    
    expression_parser_t *parser = ar__expression_parser__create(ref_expr);
    if (!parser) {
        return NULL;
    }
    
    expression_ast_t *ast = ar__expression_parser__parse_expression(parser);
    ar__expression_parser__destroy(parser);
    
    if (!ast) {
        return NULL;
    }
    
    data_t *result = _evaluate_expression_ast(mut_expr_evaluator, ast);
    ar__expression_ast__destroy(ast);
    
    return result;
}

/* Helper function to store result in memory if assignment path is provided */
static bool _store_result_if_assigned(
    data_t *mut_memory,
    const instruction_ast_t *ref_ast,
    data_t *own_result
) {
    const char *ref_result_path = ar__instruction_ast__get_function_result_path(ref_ast);
    if (!ref_result_path) {
        // No assignment, just destroy the result
        ar__data__destroy(own_result);
        return true;
    }
    
    // Get memory key path
    const char *key_path = _get_memory_key_path(ref_result_path);
    if (!key_path) {
        ar__data__destroy(own_result);
        return false;
    }
    
    // Store the result (transfers ownership)
    bool store_success = ar__data__set_map_data(mut_memory, key_path, own_result);
    if (!store_success) {
        ar__data__destroy(own_result);
        return false;
    }
    
    return true;
}

/**
 * Creates a new destroy agent instruction evaluator instance
 */
ar_destroy_agent_instruction_evaluator_t* ar_destroy_agent_instruction_evaluator__create(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory
) {
    if (!mut_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_destroy_agent_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(
        sizeof(ar_destroy_agent_instruction_evaluator_t), 
        "destroy_agent_instruction_evaluator"
    );
    if (!own_evaluator) {
        return NULL;
    }
    
    own_evaluator->mut_expr_evaluator = mut_expr_evaluator;
    own_evaluator->mut_memory = mut_memory;
    
    return own_evaluator;
}

/**
 * Destroys a destroy agent instruction evaluator instance
 */
void ar_destroy_agent_instruction_evaluator__destroy(ar_destroy_agent_instruction_evaluator_t *own_evaluator) {
    if (!own_evaluator) {
        return;
    }
    
    AR__HEAP__FREE(own_evaluator);
}

/**
 * Evaluates a destroy agent instruction using stored dependencies
 */
bool ar_destroy_agent_instruction_evaluator__evaluate(
    const ar_destroy_agent_instruction_evaluator_t *ref_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!ref_evaluator || !ref_ast) {
        return false;
    }
    
    expression_evaluator_t *mut_expr_evaluator = ref_evaluator->mut_expr_evaluator;
    data_t *mut_memory = ref_evaluator->mut_memory;
    
    // Validate AST type
    if (ar__instruction_ast__get_type(ref_ast) != INST_AST_DESTROY) {
        return false;
    }
    
    // Get function arguments
    list_t *own_args = ar__instruction_ast__get_function_args(ref_ast);
    if (!own_args) {
        return false;
    }
    
    size_t arg_count = ar__list__count(own_args);
    if (arg_count != 1) {
        ar__list__destroy(own_args);
        return false;
    }
    
    bool success = false;
    bool destroy_result = false;
    
    // Get array of arguments
    void **items = ar__list__items(own_args);
    if (!items) {
        ar__list__destroy(own_args);
        return false;
    }
    
    // Extract and evaluate agent ID argument
    const char *ref_agent_expr = (const char*)items[0];
    data_t *own_agent_id = _parse_and_evaluate_expression(mut_expr_evaluator, ref_agent_expr);
    
    if (own_agent_id && ar__data__get_type(own_agent_id) == DATA_INTEGER) {
        int64_t agent_id = (int64_t)ar__data__get_integer(own_agent_id);
        destroy_result = ar__agency__destroy_agent(agent_id);
        success = true;
    }
    
    if (own_agent_id) {
        ar__data__destroy(own_agent_id);
    }
    
    // Free the items array (but not the items themselves)
    AR__HEAP__FREE(items);
    
    // Destroy the args list
    ar__list__destroy(own_args);
    
    // Store result if assigned
    if (success && ar__instruction_ast__has_result_assignment(ref_ast)) {
        data_t *own_result = ar__data__create_integer(destroy_result ? 1 : 0);
        if (own_result) {
            _store_result_if_assigned(mut_memory, ref_ast, own_result);
        }
    }
    
    return success;
}