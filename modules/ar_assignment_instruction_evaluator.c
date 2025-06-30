/**
 * @file agerun_assignment_instruction_evaluator.c
 * @brief Implementation of the assignment instruction evaluator module
 */

#include "ar_assignment_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_expression_ast.h"
#include "ar_log.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>


/* Internal structure for the assignment instruction evaluator */
struct ar_assignment_instruction_evaluator_s {
    ar_log_t *ref_log;                           /* Borrowed reference to log instance */
    ar_expression_evaluator_t *ref_expr_evaluator;  /* Borrowed reference to expression evaluator */
    data_t *mut_memory;                          /* Mutable reference to memory map */
};

/* Constants */
static const char* MEMORY_PREFIX = "memory.";
static const size_t MEMORY_PREFIX_LEN = 7;

ar_assignment_instruction_evaluator_t* ar_assignment_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
) {
    if (!ref_log || !ref_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_assignment_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(sizeof(ar_assignment_instruction_evaluator_t), "assignment_instruction_evaluator");
    if (!own_evaluator) {
        return NULL;
    }
    
    own_evaluator->ref_log = ref_log;
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
    
    // Just free the struct, we don't own the log, expression evaluator or memory
    AR__HEAP__FREE(own_evaluator);
}

/* Helper function to log error message */
static void _log_error(ar_assignment_instruction_evaluator_t *mut_evaluator, const char *message) {
    if (message && mut_evaluator->ref_log) {
        ar_log__error(mut_evaluator->ref_log, message);
    }
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
        _log_error(mut_evaluator, "Assignment target must start with 'memory.'");
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
        own_value = ar_data__shallow_copy(result);
        if (!own_value) {
            _log_error(mut_evaluator, "Cannot assign value with nested containers (no deep copy support)");
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
