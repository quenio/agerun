/**
 * @file agerun_send_instruction_evaluator.c
 * @brief Implementation of the send instruction evaluator module
 */

#include "agerun_send_instruction_evaluator.h"
#include "agerun_heap.h"
#include "agerun_expression_parser.h"
#include "agerun_expression_ast.h"
#include "agerun_agency.h"
#include "agerun_list.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

/* Forward declaration of legacy function */
bool ar_send_instruction_evaluator__evaluate_legacy(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);

/**
 * Internal structure for send instruction evaluator
 */
struct ar_send_instruction_evaluator_s {
    expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
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
 * Creates a new send instruction evaluator
 */
send_instruction_evaluator_t* ar_send_instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
) {
    if (!ref_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    send_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(
        sizeof(send_instruction_evaluator_t),
        "send_instruction_evaluator"
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
 * Destroys a send instruction evaluator
 */
void ar_send_instruction_evaluator__destroy(
    send_instruction_evaluator_t *own_evaluator
) {
    if (!own_evaluator) {
        return;
    }
    
    // Note: We don't destroy the dependencies as they are borrowed references
    AR__HEAP__FREE(own_evaluator);
}

/**
 * Evaluates a send instruction AST node using stored dependencies
 */
bool ar_send_instruction_evaluator__evaluate(
    send_instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the legacy function with stored dependencies
    return ar_send_instruction_evaluator__evaluate_legacy(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        ref_ast
    );
}

/**
 * Evaluates a send instruction AST node (legacy interface)
 */
bool ar_send_instruction_evaluator__evaluate_legacy(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
) {
    if (!mut_expr_evaluator || !mut_memory || !ref_ast) {
        return false;
    }
    
    // Verify this is a send AST node
    if (ar__instruction_ast__get_type(ref_ast) != INST_AST_SEND) {
        return false;
    }
    
    // Get function arguments
    list_t *own_args = NULL;
    void **items = _extract_function_args(ref_ast, 2, &own_args);
    if (!items) {
        return false;
    }
    
    const char *ref_agent_id_expr = (const char*)items[0];
    const char *ref_message_expr = (const char*)items[1];
    
    if (!ref_agent_id_expr || !ref_message_expr) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    // Parse and evaluate agent ID expression
    expression_parser_t *parser = ar__expression_parser__create(ref_agent_id_expr);
    if (!parser) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    expression_ast_t *agent_id_ast = ar__expression_parser__parse_expression(parser);
    ar__expression_parser__destroy(parser);
    
    if (!agent_id_ast) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    data_t *own_agent_id_data = _evaluate_expression_ast(mut_expr_evaluator, agent_id_ast);
    ar__expression_ast__destroy(agent_id_ast);
    
    if (!own_agent_id_data) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    // Extract agent ID as integer
    int64_t agent_id = 0;
    if (ar__data__get_type(own_agent_id_data) == DATA_INTEGER) {
        agent_id = ar__data__get_integer(own_agent_id_data);
    }
    ar__data__destroy(own_agent_id_data);
    
    // Parse and evaluate message expression
    parser = ar__expression_parser__create(ref_message_expr);
    if (!parser) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    expression_ast_t *message_ast = ar__expression_parser__parse_expression(parser);
    ar__expression_parser__destroy(parser);
    
    if (!message_ast) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    data_t *own_message = _evaluate_expression_ast(mut_expr_evaluator, message_ast);
    ar__expression_ast__destroy(message_ast);
    
    if (!own_message) {
        _cleanup_function_args(items, own_args);
        return false;
    }
    
    // Clean up items array and args list (we're done with them)
    _cleanup_function_args(items, own_args);
    
    // Send the message
    bool send_result;
    if (agent_id == 0) {
        // Special case: agent_id 0 is a no-op that always returns true
        ar__data__destroy(own_message);
        send_result = true;
    } else {
        // Send message (ownership transferred to ar__agency__send_to_agent)
        send_result = ar__agency__send_to_agent(agent_id, own_message);
    }
    
    // Handle result assignment if present
    const char *ref_result_path = ar__instruction_ast__get_function_result_path(ref_ast);
    if (ref_result_path) {
        // Get memory key path
        const char *key_path = _get_memory_key_path(ref_result_path);
        if (!key_path) {
            return false;
        }
        
        // Create result value (true = 1, false = 0)
        data_t *own_result = ar__data__create_integer(send_result ? 1 : 0);
        bool store_success = ar__data__set_map_data(mut_memory, key_path, own_result);
        if (!store_success) {
            ar__data__destroy(own_result);
        }
        
        // For assignments, return true to indicate the instruction succeeded
        return true;
    }
    
    return send_result;
}