/**
 * @file ar_destroy_agent_instruction_evaluator.c
 * @brief Implementation of the destroy agent instruction evaluator module
 */

#include "ar_destroy_agent_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_expression_ast.h"
#include "ar_expression_evaluator.h"
#include "ar_agency.h"
#include "ar_list.h"
#include "ar_log.h"
#include "ar_memory_accessor.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>



/* Opaque struct definition */
struct ar_destroy_agent_instruction_evaluator_s {
    ar_log_t *ref_log;                           /* Borrowed reference to log instance */
    ar_expression_evaluator_t *mut_expr_evaluator;
    ar_data_t *mut_memory;
};

/* Helper function to log error message */
static void _log_error(ar_destroy_agent_instruction_evaluator_t *mut_evaluator, const char *message) {
    if (message && mut_evaluator->ref_log) {
        ar_log__error(mut_evaluator->ref_log, message);
    }
}





/* Helper function to store result in memory if assignment path is provided */
static bool _store_result_if_assigned(
    ar_data_t *mut_memory,
    const ar_instruction_ast_t *ref_ast,
    ar_data_t *own_result
) {
    const char *ref_result_path = ar_instruction_ast__get_function_result_path(ref_ast);
    if (!ref_result_path) {
        // No assignment, just destroy the result
        ar_data__destroy(own_result);
        return true;
    }
    
    // Get memory key path
    const char *key_path = ar_memory_accessor__get_key(ref_result_path);
    if (!key_path) {
        ar_data__destroy(own_result);
        return false;
    }
    
    // Store the result (transfers ownership)
    bool store_success = ar_data__set_map_data(mut_memory, key_path, own_result);
    if (!store_success) {
        ar_data__destroy(own_result);
        return false;
    }
    
    return true;
}

/**
 * Creates a new destroy agent instruction evaluator instance
 */
ar_destroy_agent_instruction_evaluator_t* ar_destroy_agent_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *mut_expr_evaluator,
    ar_data_t *mut_memory
) {
    if (!ref_log || !mut_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_destroy_agent_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(
        sizeof(ar_destroy_agent_instruction_evaluator_t), 
        "destroy_agent_instruction_evaluator"
    );
    if (!own_evaluator) {
        return NULL;
    }
    
    own_evaluator->ref_log = ref_log;
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
    ar_destroy_agent_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Clear any previous error
    _log_error(mut_evaluator, NULL);
    
    ar_expression_evaluator_t *mut_expr_evaluator = mut_evaluator->mut_expr_evaluator;
    ar_data_t *mut_memory = mut_evaluator->mut_memory;
    
    // Validate AST type
    if (ar_instruction_ast__get_type(ref_ast) != AR_INSTRUCTION_AST_TYPE__DESTROY_AGENT) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const ar_list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(ref_ast);
    if (!ref_arg_asts) {
        return false;
    }
    
    // Verify we have exactly 1 argument
    if (ar_list__count(ref_arg_asts) != 1) {
        return false;
    }
    
    // Get the argument ASTs array
    void **items = ar_list__items(ref_arg_asts);
    if (!items) {
        return false;
    }
    
    const ar_expression_ast_t *ref_agent_id_ast = (const ar_expression_ast_t*)items[0];
    if (!ref_agent_id_ast) {
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Evaluate the agent ID expression AST using public method
    ar_data_t *agent_id_result = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_agent_id_ast);
    
    // Check if we need to make a copy (if result is owned by memory/context)
    ar_data_t *own_agent_id = NULL;
    if (agent_id_result) {
        if (ar_data__hold_ownership(agent_id_result, mut_expr_evaluator)) {
            // We can claim ownership - it's an unowned value (literal or operation result)
            ar_data__transfer_ownership(agent_id_result, mut_expr_evaluator);
            own_agent_id = agent_id_result;
        } else {
            // It's owned by someone else (memory access) - we need to make a copy
            own_agent_id = ar_data__shallow_copy(agent_id_result);
            if (!own_agent_id) {
                _log_error(mut_evaluator, "Cannot destroy agent with nested containers in agent ID (no deep copy support)");
                AR__HEAP__FREE(items);
                return false;
            }
        }
    }
    
    bool success = false;
    bool destroy_result = false;
    
    if (own_agent_id && ar_data__get_type(own_agent_id) == AR_DATA_TYPE__INTEGER) {
        int64_t agent_id = (int64_t)ar_data__get_integer(own_agent_id);
        destroy_result = ar_agency__destroy_agent(agent_id);
        success = true;
    }
    
    if (own_agent_id) {
        ar_data__destroy(own_agent_id);
    }
    
    // Free the items array
    AR__HEAP__FREE(items);
    
    // Store result if assigned
    if (success && ar_instruction_ast__has_result_assignment(ref_ast)) {
        ar_data_t *own_result = ar_data__create_integer(destroy_result ? 1 : 0);
        if (own_result) {
            _store_result_if_assigned(mut_memory, ref_ast, own_result);
        }
    }
    
    return success;
}
