/**
 * @file agerun_method_instruction_evaluator.c
 * @brief Implementation of the method instruction evaluator module
 */

#include "ar_method_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_expression_ast.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_log.h"
#include "ar_memory_accessor.h"
#include <string.h>
#include <stdio.h>

/* Forward declaration of legacy function */
bool ar_method_instruction_evaluator__evaluate_legacy(
    ar_expression_evaluator_t *mut_expr_evaluator,
    ar_data_t *mut_memory,
    const ar_instruction_ast_t *ref_ast
);

/**
 * Internal structure for method instruction evaluator
 * 
 * Note: This struct does not store a methodology reference because
 * ar_methodology__register_method() uses a global singleton internally.
 */
struct ar_method_instruction_evaluator_s {
    ar_log_t *ref_log;                           /* Borrowed reference to log instance */
    ar_expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    ar_data_t *mut_memory;                          /* Memory map (mutable reference) */
};

/**
 * Creates a new method instruction evaluator instance
 */
ar_method_instruction_evaluator_t* ar_method_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    ar_data_t *mut_memory
) {
    // Validate required parameters
    if (ref_log == NULL || ref_expr_evaluator == NULL || mut_memory == NULL) {
        return NULL;
    }
    
    // Allocate evaluator structure
    ar_method_instruction_evaluator_t *evaluator = AR__HEAP__MALLOC(
        sizeof(ar_method_instruction_evaluator_t), 
        "method_instruction_evaluator"
    );
    if (evaluator == NULL) {
        return NULL;
    }
    
    // Initialize fields
    evaluator->ref_log = ref_log;
    evaluator->ref_expr_evaluator = ref_expr_evaluator;
    evaluator->mut_memory = mut_memory;
    
    return evaluator;
}

/**
 * Destroys a method instruction evaluator instance
 */
void ar_method_instruction_evaluator__destroy(
    ar_method_instruction_evaluator_t *own_evaluator
) {
    if (own_evaluator == NULL) {
        return;
    }
    
    // Free the evaluator structure
    AR__HEAP__FREE(own_evaluator);
}


/* Helper function to log error message */
static void _log_error(ar_method_instruction_evaluator_t *mut_evaluator, const char *message) {
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

/* Helper function to evaluate three string arguments from a function call */
static bool _evaluate_three_string_args(
    ar_method_instruction_evaluator_t *mut_evaluator,
    ar_expression_evaluator_t *mut_expr_evaluator,
    const ar_instruction_ast_t *ref_ast,
    size_t expected_arg_count,
    ar_data_t **out_arg1,
    ar_data_t **out_arg2,
    ar_data_t **out_arg3
) {
    // Get pre-parsed expression ASTs for arguments
    const list_t *ref_arg_asts = ar_instruction_ast__get_function_arg_asts(ref_ast);
    if (!ref_arg_asts) {
        return false;
    }
    
    // Verify we have exactly the expected number of arguments
    if (ar_list__count(ref_arg_asts) != expected_arg_count) {
        return false;
    }
    
    // Get the argument ASTs array
    void **items = ar_list__items(ref_arg_asts);
    if (!items) {
        return false;
    }
    
    const ar_expression_ast_t *ref_ast1 = (const ar_expression_ast_t*)items[0];
    const ar_expression_ast_t *ref_ast2 = (const ar_expression_ast_t*)items[1];
    const ar_expression_ast_t *ref_ast3 = (const ar_expression_ast_t*)items[2];
    
    if (!ref_ast1 || !ref_ast2 || !ref_ast3) {
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Evaluate expression ASTs using public method
    ar_data_t *result1 = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_ast1);
    ar_data_t *result2 = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_ast2);
    ar_data_t *result3 = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_ast3);
    
    AR__HEAP__FREE(items);
    
    // Handle ownership for arg1
    if (result1) {
        if (ar_data__hold_ownership(result1, mut_evaluator)) {
            ar_data__transfer_ownership(result1, mut_evaluator);
            *out_arg1 = result1;
        } else {
            *out_arg1 = ar_data__shallow_copy(result1);
            if (!*out_arg1) {
                _log_error(mut_evaluator, "Cannot create method with nested containers in argument 1 (no deep copy support)");
                if (result2) {
                    if (ar_data__hold_ownership(result2, mut_evaluator)) {
                        ar_data__transfer_ownership(result2, mut_evaluator);
                        ar_data__destroy(result2);
                    }
                }
                if (result3) {
                    if (ar_data__hold_ownership(result3, mut_evaluator)) {
                        ar_data__transfer_ownership(result3, mut_evaluator);
                        ar_data__destroy(result3);
                    }
                }
                return false;
            }
        }
    } else {
        *out_arg1 = NULL;
    }
    
    // Handle ownership for arg2
    if (result2) {
        if (ar_data__hold_ownership(result2, mut_evaluator)) {
            ar_data__transfer_ownership(result2, mut_evaluator);
            *out_arg2 = result2;
        } else {
            *out_arg2 = ar_data__shallow_copy(result2);
            if (!*out_arg2) {
                _log_error(mut_evaluator, "Cannot create method with nested containers in argument 2 (no deep copy support)");
                ar_data__destroy(*out_arg1);
                if (result3) {
                    if (ar_data__hold_ownership(result3, mut_evaluator)) {
                        ar_data__transfer_ownership(result3, mut_evaluator);
                        ar_data__destroy(result3);
                    }
                }
                return false;
            }
        }
    } else {
        *out_arg2 = NULL;
    }
    
    // Handle ownership for arg3
    if (result3) {
        if (ar_data__hold_ownership(result3, mut_evaluator)) {
            ar_data__transfer_ownership(result3, mut_evaluator);
            *out_arg3 = result3;
        } else {
            *out_arg3 = ar_data__shallow_copy(result3);
            if (!*out_arg3) {
                _log_error(mut_evaluator, "Cannot create method with nested containers in argument 3 (no deep copy support)");
                ar_data__destroy(*out_arg1);
                ar_data__destroy(*out_arg2);
                return false;
            }
        }
    } else {
        *out_arg3 = NULL;
    }
    
    // Validate all arguments are strings
    if (*out_arg1 && *out_arg2 && *out_arg3 &&
        ar_data__get_type(*out_arg1) == DATA_STRING &&
        ar_data__get_type(*out_arg2) == DATA_STRING &&
        ar_data__get_type(*out_arg3) == DATA_STRING) {
        return true;
    }
    
    return false;
}

bool ar_method_instruction_evaluator__evaluate(
    ar_method_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Clear any previous error
    _log_error(mut_evaluator, NULL);
    
    // Extract dependencies from the evaluator instance
    ar_expression_evaluator_t *mut_expr_evaluator = mut_evaluator->ref_expr_evaluator;
    ar_data_t *mut_memory = mut_evaluator->mut_memory;
    
    if (!mut_expr_evaluator || !mut_memory) {
        return false;
    }
    
    // Validate AST type
    if (ar_instruction_ast__get_type(ref_ast) != AR_INST__METHOD) {
        return false;
    }
    
    // Evaluate three string arguments
    ar_data_t *own_method_name = NULL;
    ar_data_t *own_instructions = NULL;
    ar_data_t *own_version = NULL;
    
    bool args_valid = _evaluate_three_string_args(
        mut_evaluator, mut_expr_evaluator, ref_ast, 3,
        &own_method_name, &own_instructions, &own_version
    );
    
    bool success = false;
    
    if (args_valid) {
        const char *method_name = ar_data__get_string(own_method_name);
        const char *instructions = ar_data__get_string(own_instructions);
        const char *version = ar_data__get_string(own_version);
        
        // Create and register the method
        method_t *own_method = ar_method__create_with_log(method_name, instructions, version, mut_evaluator->ref_log);
        if (own_method) {
            ar_methodology__register_method(own_method);
            // Ownership transferred to methodology
            own_method = NULL;
            success = true;
        }
    }
    
    // Clean up evaluated arguments
    if (own_method_name) ar_data__destroy(own_method_name);
    if (own_instructions) ar_data__destroy(own_instructions);
    if (own_version) ar_data__destroy(own_version);
    
    // Store result if assigned
    if (ar_instruction_ast__has_result_assignment(ref_ast)) {
        ar_data_t *own_result = ar_data__create_integer(success ? 1 : 0);
        if (own_result) {
            _store_result_if_assigned(mut_memory, ref_ast, own_result);
        }
    }
    
    return success;
}

