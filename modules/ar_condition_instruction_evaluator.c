/**
 * @file agerun_condition_instruction_evaluator.c
 * @brief Implementation of the condition instruction evaluator module
 */

#include "ar_condition_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_expression_ast.h"
#include "ar_list.h"
#include "ar_log.h"
#include "ar_memory_accessor.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>


/**
 * Internal structure for condition instruction evaluator
 */
struct ar_condition_instruction_evaluator_s {
    ar_log_t *ref_log;                           /* Borrowed reference to log instance */
    ar_expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    ar_data_t *mut_memory;                          /* Memory map (mutable reference) */
};


/* Helper function to log error message */
static void _log_error(ar_condition_instruction_evaluator_t *mut_evaluator, const char *message) {
    if (message && mut_evaluator->ref_log) {
        ar_log__error(mut_evaluator->ref_log, message);
    }
}





/**
 * Creates a new condition instruction evaluator
 */
ar_condition_instruction_evaluator_t* ar_condition_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    ar_data_t *mut_memory
) {
    if (!ref_log || !ref_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_condition_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(
        sizeof(ar_condition_instruction_evaluator_t),
        "condition_instruction_evaluator"
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
    
    // Clear any previous error
    _log_error(mut_evaluator, NULL);
    
    // Verify this is an if AST node
    if (ar_instruction_ast__get_type(ref_ast) != AR_INSTRUCTION_AST_TYPE__IF) {
        return false;
    }
    
    // Get pre-parsed expression ASTs for arguments
    const ar_list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(ref_ast);
    if (!ref_arg_asts) {
        return false;
    }
    
    // Verify we have exactly 3 arguments
    if (ar_list__count(ref_arg_asts) != 3) {
        return false;
    }
    
    // Get the argument ASTs array
    void **items = ar_list__items(ref_arg_asts);
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
    ar_data_t *condition_result = ar_expression_evaluator__evaluate(mut_evaluator->ref_expr_evaluator, ref_condition_ast);
    if (!condition_result) {
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Check condition value (0 is false, non-zero is true)
    bool condition_is_true = false;
    if (ar_data__get_type(condition_result) == AR_DATA_TYPE__INTEGER) {
        condition_is_true = (ar_data__get_integer(condition_result) != 0);
    }
    
    // We only need the value, not the data itself
    if (ar_data__hold_ownership(condition_result, mut_evaluator)) {
        ar_data__transfer_ownership(condition_result, mut_evaluator);
        ar_data__destroy(condition_result);
    }
    
    // Select which expression AST to evaluate based on condition
    const ar_expression_ast_t *ref_ast_to_eval = condition_is_true ? ref_true_ast : ref_false_ast;
    
    // Evaluate the selected expression AST
    ar_data_t *result = ar_expression_evaluator__evaluate(mut_evaluator->ref_expr_evaluator, ref_ast_to_eval);
    
    // Clean up the items array as we're done with it
    AR__HEAP__FREE(items);
    
    if (!result) {
        return false;
    }
    
    // Handle result assignment if present
    const char *ref_result_path = ar_instruction_ast__get_function_result_path(ref_ast);
    if (ref_result_path) {
        // Get memory key path
        const char *key_path = ar_memory_accessor__get_key(ref_result_path);
        if (!key_path) {
            // Clean up result if we can
            if (ar_data__hold_ownership(result, mut_evaluator)) {
                ar_data__transfer_ownership(result, mut_evaluator);
                ar_data__destroy(result);
            }
            return false;
        }
        
        // Get ownership of result for storing
        ar_data_t *own_result;
        if (ar_data__hold_ownership(result, mut_evaluator)) {
            // We can claim ownership
            ar_data__transfer_ownership(result, mut_evaluator);
            own_result = result;
        } else {
            // Need to make a copy
            own_result = ar_data__shallow_copy(result);
            if (!own_result) {
                _log_error(mut_evaluator, "Cannot assign value with nested containers (no deep copy support)");
                return false;
            }
        }
        
        // Store the result value (transfers ownership)
        bool store_success = ar_data__set_map_data(mut_evaluator->mut_memory, key_path, own_result);
        if (!store_success) {
            ar_data__destroy(own_result);
        }
        
        // For assignments, return true to indicate the instruction succeeded
        return true;
    } else {
        // No assignment, just return success (expression was evaluated for side effects)
        if (ar_data__hold_ownership(result, mut_evaluator)) {
            ar_data__transfer_ownership(result, mut_evaluator);
            ar_data__destroy(result);
        }
        return true;
    }
}
