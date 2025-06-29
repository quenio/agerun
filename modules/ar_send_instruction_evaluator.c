/**
 * @file agerun_send_instruction_evaluator.c
 * @brief Implementation of the send instruction evaluator module
 */

#include "ar_send_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_expression_ast.h"
#include "ar_agency.h"
#include "ar_list.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>


/**
 * Internal structure for send instruction evaluator
 */
struct ar_send_instruction_evaluator_s {
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
 * Creates a new send instruction evaluator
 */
ar_send_instruction_evaluator_t* ar_send_instruction_evaluator__create(
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
) {
    if (!ref_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_send_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(
        sizeof(ar_send_instruction_evaluator_t),
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
    ar_send_instruction_evaluator_t *own_evaluator
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
    ar_send_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Verify this is a send AST node
    if (ar_instruction_ast__get_type(ref_ast) != AR_INST__SEND) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(ref_ast);
    if (!ref_arg_asts) {
        return false;
    }
    
    // Verify we have exactly 2 arguments
    if (ar_list__count(ref_arg_asts) != 2) {
        return false;
    }
    
    // Get the argument ASTs array
    void **items = ar_list__items(ref_arg_asts);
    if (!items) {
        return false;
    }
    
    const ar_expression_ast_t *ref_agent_id_ast = (const ar_expression_ast_t*)items[0];
    const ar_expression_ast_t *ref_message_ast = (const ar_expression_ast_t*)items[1];
    
    if (!ref_agent_id_ast || !ref_message_ast) {
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Evaluate agent ID expression
    data_t *agent_id_result = ar_expression_evaluator__evaluate(mut_evaluator->ref_expr_evaluator, ref_agent_id_ast);
    if (!agent_id_result) {
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Extract agent ID as integer
    int64_t agent_id = 0;
    if (ar_data__get_type(agent_id_result) == DATA_INTEGER) {
        agent_id = ar_data__get_integer(agent_id_result);
    }
    
    // We only need the value, not the data itself
    // Check if we can destroy it (unowned) or if it's a reference
    if (ar_data__hold_ownership(agent_id_result, mut_evaluator)) {
        ar_data__transfer_ownership(agent_id_result, mut_evaluator);
        ar_data__destroy(agent_id_result);
    }
    
    // Evaluate message expression
    data_t *message_result = ar_expression_evaluator__evaluate(mut_evaluator->ref_expr_evaluator, ref_message_ast);
    
    // Clean up the items array as we're done with it
    AR__HEAP__FREE(items);
    
    if (!message_result) {
        return false;
    }
    
    // Get ownership of message for sending
    data_t *own_message;
    if (ar_data__hold_ownership(message_result, mut_evaluator)) {
        // We can claim ownership - it's an unowned value
        ar_data__transfer_ownership(message_result, mut_evaluator);
        own_message = message_result;
    } else {
        // It's owned by someone else - we need to make a copy
        own_message = _copy_data_value(message_result);
        if (!own_message) {
            return false;
        }
    }
    
    // Send the message
    bool send_result;
    if (agent_id == 0) {
        // Special case: agent_id 0 is a no-op that always returns true
        ar_data__destroy(own_message);
        send_result = true;
    } else {
        // Send message (ownership transferred to ar_agency__send_to_agent)
        send_result = ar_agency__send_to_agent(agent_id, own_message);
    }
    
    // Handle result assignment if present
    const char *ref_result_path = ar_instruction_ast__get_function_result_path(ref_ast);
    if (ref_result_path) {
        // Get memory key path
        const char *key_path = _get_memory_key_path(ref_result_path);
        if (!key_path) {
            return false;
        }
        
        // Create result value (true = 1, false = 0)
        data_t *own_result = ar_data__create_integer(send_result ? 1 : 0);
        bool store_success = ar_data__set_map_data(mut_evaluator->mut_memory, key_path, own_result);
        if (!store_success) {
            ar_data__destroy(own_result);
        }
        
        // For assignments, return true to indicate the instruction succeeded
        return true;
    }
    
    return send_result;
}