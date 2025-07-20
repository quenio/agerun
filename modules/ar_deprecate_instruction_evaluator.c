/**
 * @file ar_deprecate_instruction_evaluator.c
 * @brief Implementation of the deprecate instruction evaluator module
 */

#include "ar_deprecate_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_frame.h"
#include "ar_expression_ast.h"
#include "ar_expression_evaluator.h"
#include "ar_agency.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_list.h"
#include "ar_log.h"
#include "ar_data.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>



/* Opaque struct definition */
struct ar_deprecate_instruction_evaluator_s {
    ar_log_t *ref_log;                           /* Borrowed reference to log instance */
    ar_expression_evaluator_t *ref_expr_evaluator; /* Borrowed reference to expression evaluator */
};

/* Helper function to log error message */
static void _log_error(ar_deprecate_instruction_evaluator_t *mut_evaluator, const char *message) {
    if (message && mut_evaluator->ref_log) {
        ar_log__error(mut_evaluator->ref_log, message);
    }
}






/**
 * Creates a new deprecate instruction evaluator instance
 */
ar_deprecate_instruction_evaluator_t* ar_deprecate_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
) {
    if (!ref_log || !ref_expr_evaluator) {
        return NULL;
    }
    
    ar_deprecate_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(
        sizeof(ar_deprecate_instruction_evaluator_t), 
        "deprecate_instruction_evaluator"
    );
    if (!own_evaluator) {
        return NULL;
    }
    
    own_evaluator->ref_log = ref_log;
    own_evaluator->ref_expr_evaluator = ref_expr_evaluator;
    
    return own_evaluator;
}

/**
 * Destroys a deprecate instruction evaluator instance
 */
void ar_deprecate_instruction_evaluator__destroy(ar_deprecate_instruction_evaluator_t *own_evaluator) {
    if (!own_evaluator) {
        return;
    }
    
    AR__HEAP__FREE(own_evaluator);
}

/**
 * Evaluates a deprecate instruction using frame-based execution
 */
bool ar_deprecate_instruction_evaluator__evaluate(
    ar_deprecate_instruction_evaluator_t *mut_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_frame || !ref_ast) {
        return false;
    }
    
    // Clear any previous error
    _log_error(mut_evaluator, NULL);
    
    ar_expression_evaluator_t *mut_expr_evaluator = mut_evaluator->ref_expr_evaluator;
    ar_data_t *mut_memory = ar_frame__get_memory(ref_frame);
    if (!mut_memory) {
        return false;
    }
    
    // Validate AST type
    if (ar_instruction_ast__get_type(ref_ast) != AR_INSTRUCTION_AST_TYPE__DEPRECATE) {
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
    
    const ar_expression_ast_t *ref_name_ast = (const ar_expression_ast_t*)items[0];
    const ar_expression_ast_t *ref_version_ast = (const ar_expression_ast_t*)items[1];
    
    if (!ref_name_ast || !ref_version_ast) {
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Evaluate expression ASTs using public method
    ar_data_t *name_result = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_frame, ref_name_ast);
    ar_data_t *version_result = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_frame, ref_version_ast);
    
    // Handle ownership for name
    ar_data_t *own_name = NULL;
    if (name_result) {
        own_name = ar_data__claim_or_copy(name_result, mut_evaluator);
        if (!own_name) {
            _log_error(mut_evaluator, "Cannot deprecate method with nested containers in name (no deep copy support)");
            AR__HEAP__FREE(items);
            return false;
        }
    }
    
    // Handle ownership for version
    ar_data_t *own_version = NULL;
    if (version_result) {
        own_version = ar_data__claim_or_copy(version_result, mut_evaluator);
        if (!own_version) {
            _log_error(mut_evaluator, "Cannot deprecate method with nested containers in version (no deep copy support)");
            if (own_name) ar_data__destroy(own_name);
            AR__HEAP__FREE(items);
            return false;
        }
    }
    
    bool success = false;
    bool destroy_result = false;
    
    if (own_name && own_version &&
        ar_data__get_type(own_name) == AR_DATA_TYPE__STRING &&
        ar_data__get_type(own_version) == AR_DATA_TYPE__STRING) {
        
        const char *method_name = ar_data__get_string(own_name);
        const char *method_version = ar_data__get_string(own_version);
        
        // Get the method to check if it exists
        ar_method_t *ref_method = ar_methodology__get_method(method_name, method_version);
        if (ref_method) {
            // Unregister the method (deprecate only unregisters, doesn't destroy agents)
            destroy_result = ar_methodology__unregister_method(method_name, method_version);
            success = true;
        } else {
            // Method doesn't exist
            success = true;
            destroy_result = false;
        }
    }
    
    if (own_name) ar_data__destroy(own_name);
    if (own_version) ar_data__destroy(own_version);
    
    // Free the items array
    AR__HEAP__FREE(items);
    
    // Store result if assigned
    if (success && ar_instruction_ast__has_result_assignment(ref_ast)) {
        ar_data_t *own_result = ar_data__create_integer(destroy_result ? 1 : 0);
        if (own_result) {
            const char *ref_result_path = ar_instruction_ast__get_function_result_path(ref_ast);
            if (!ar_data__set_map_data_if_root_matched(mut_memory, "memory", ref_result_path, own_result)) {
                ar_data__destroy(own_result);
            }
        }
    }
    
    return success;
}
