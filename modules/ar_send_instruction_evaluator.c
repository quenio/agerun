/**
 * @file agerun_send_instruction_evaluator.c
 * @brief Implementation of the send instruction evaluator module
 */

#include "ar_send_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_expression_ast.h"
#include "ar_agency.h"
#include "ar_list.h"
#include "ar_log.h"
#include "ar_memory_accessor.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>


/**
 * Internal structure for send instruction evaluator
 */
struct ar_send_instruction_evaluator_s {
    ar_log_t *ref_log;                           /* Borrowed reference to log instance */
    ar_expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    ar_data_t *mut_memory;                          /* Memory map (mutable reference) */
};


/* Helper function to log error message */
static void _log_error(ar_send_instruction_evaluator_t *mut_evaluator, const char *message) {
    if (message && mut_evaluator->ref_log) {
        ar_log__error(mut_evaluator->ref_log, message);
    }
}





/**
 * Creates a new send instruction evaluator
 */
ar_send_instruction_evaluator_t* ar_send_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    ar_data_t *mut_memory
) {
    if (!ref_log || !ref_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_send_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(
        sizeof(ar_send_instruction_evaluator_t),
        "send_instruction_evaluator"
    );
    if (!own_evaluator) {
        return NULL;
    }
    
    own_evaluator->ref_log = ref_log;
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
    
    // Clear any previous error
    _log_error(mut_evaluator, NULL);
    
    // Verify this is a send AST node
    if (ar_instruction_ast__get_type(ref_ast) != AR_INST__SEND) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const ar_list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(ref_ast);
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
    ar_data_t *agent_id_result = ar_expression_evaluator__evaluate(mut_evaluator->ref_expr_evaluator, ref_agent_id_ast);
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
    ar_data_t *message_result = ar_expression_evaluator__evaluate(mut_evaluator->ref_expr_evaluator, ref_message_ast);
    
    // Clean up the items array as we're done with it
    AR__HEAP__FREE(items);
    
    if (!message_result) {
        return false;
    }
    
    // Get ownership of message for sending
    ar_data_t *own_message;
    if (ar_data__hold_ownership(message_result, mut_evaluator)) {
        // We can claim ownership - it's an unowned value
        ar_data__transfer_ownership(message_result, mut_evaluator);
        own_message = message_result;
    } else {
        // It's owned by someone else - we need to make a copy
        own_message = ar_data__shallow_copy(message_result);
        if (!own_message) {
            _log_error(mut_evaluator, "Cannot send message with nested containers (no deep copy support)");
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
        const char *key_path = ar_memory_accessor__get_key(ref_result_path);
        if (!key_path) {
            return false;
        }
        
        // Create result value (true = 1, false = 0)
        ar_data_t *own_result = ar_data__create_integer(send_result ? 1 : 0);
        bool store_success = ar_data__set_map_data(mut_evaluator->mut_memory, key_path, own_result);
        if (!store_success) {
            ar_data__destroy(own_result);
        }
        
        // For assignments, return true to indicate the instruction succeeded
        return true;
    }
    
    return send_result;
}
