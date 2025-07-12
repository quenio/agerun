/**
 * @file ar_agent_instruction_evaluator.c
 * @brief Implementation of the agent instruction evaluator module
 */

#include "ar_agent_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_expression_ast.h"
#include "ar_agency.h"
#include "ar_method.h"
#include "ar_methodology.h"
#include "ar_list.h"
#include "ar_log.h"
#include "ar_memory_accessor.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>


/* Opaque struct definition */
struct ar_agent_instruction_evaluator_s {
    ar_log_t *ref_log;                           /* Borrowed reference to log instance */
    ar_expression_evaluator_t *mut_expr_evaluator;
    ar_data_t *mut_memory;
};

ar_agent_instruction_evaluator_t* ar_agent_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *mut_expr_evaluator,
    ar_data_t *mut_memory
) {
    if (!ref_log || !mut_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_agent_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(sizeof(ar_agent_instruction_evaluator_t), "agent_instruction_evaluator");
    if (!own_evaluator) {
        return NULL;
    }
    
    own_evaluator->ref_log = ref_log;
    own_evaluator->mut_expr_evaluator = mut_expr_evaluator;
    own_evaluator->mut_memory = mut_memory;
    
    return own_evaluator;
}

void ar_agent_instruction_evaluator__destroy(ar_agent_instruction_evaluator_t *own_evaluator) {
    if (!own_evaluator) {
        return;
    }
    
    AR__HEAP__FREE(own_evaluator);
}


/* Helper function to log error message */
static void _log_error(ar_agent_instruction_evaluator_t *mut_evaluator, const char *message) {
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


bool ar_agent_instruction_evaluator__evaluate(
    ar_agent_instruction_evaluator_t *mut_evaluator,
    const ar_data_t *ref_context __attribute__((unused)),
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
    if (ar_instruction_ast__get_type(ref_ast) != AR_INSTRUCTION_AST_TYPE__AGENT) {
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
    
    const ar_expression_ast_t *ref_method_ast = (const ar_expression_ast_t*)items[0];
    const ar_expression_ast_t *ref_version_ast = (const ar_expression_ast_t*)items[1];
    const ar_expression_ast_t *ref_context_ast = (const ar_expression_ast_t*)items[2];
    
    if (!ref_method_ast || !ref_version_ast || !ref_context_ast) {
        AR__HEAP__FREE(items);
        return false;
    }
    
    // Evaluate expression ASTs using public method
    ar_data_t *method_result = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_method_ast);
    ar_data_t *version_result = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_version_ast);
    ar_data_t *context_result = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_context_ast);
    
    // Handle ownership for method name
    ar_data_t *own_method_name = NULL;
    if (method_result) {
        if (ar_data__hold_ownership(method_result, mut_expr_evaluator)) {
            ar_data__transfer_ownership(method_result, mut_expr_evaluator);
            own_method_name = method_result;
        } else {
            own_method_name = ar_data__shallow_copy(method_result);
            if (!own_method_name) {
                _log_error(mut_evaluator, "Cannot create agent with nested containers in method name (no deep copy support)");
                AR__HEAP__FREE(items);
                return false;
            }
        }
    }
    
    // Handle ownership for version
    ar_data_t *own_version = NULL;
    if (version_result) {
        if (ar_data__hold_ownership(version_result, mut_expr_evaluator)) {
            ar_data__transfer_ownership(version_result, mut_expr_evaluator);
            own_version = version_result;
        } else {
            own_version = ar_data__shallow_copy(version_result);
            if (!own_version) {
                _log_error(mut_evaluator, "Cannot create agent with nested containers in version (no deep copy support)");
                if (own_method_name) ar_data__destroy(own_method_name);
                AR__HEAP__FREE(items);
                return false;
            }
        }
    }
    
    // For context, use the reference directly - agency expects a borrowed reference
    const ar_data_t *ref_context_data = context_result;
    
    AR__HEAP__FREE(items);
    
    int64_t agent_id = 0;
    bool success = false;
    
    // Validate method name and version are strings
    if (own_method_name && own_version &&
        ar_data__get_type(own_method_name) == AR_DATA_TYPE__STRING &&
        ar_data__get_type(own_version) == AR_DATA_TYPE__STRING) {
        
        // Validate context - must be a map (since parser requires 3 args)
        bool context_valid = false;
        if (ref_context_data && ar_data__get_type(ref_context_data) == AR_DATA_TYPE__MAP) {
            context_valid = true;
        }
        
        if (context_valid) {
            const char *method_name = ar_data__get_string(own_method_name);
            const char *version = ar_data__get_string(own_version);
            
            // Check if method exists
            ar_method_t *ref_method = ar_methodology__get_method(method_name, version);
            if (ref_method) {
                // Create the agent - context is borrowed, not owned
                agent_id = ar_agency__create_agent(method_name, version, ref_context_data);
                if (agent_id > 0) {
                    success = true;
                }
            }
        }
    }
    
    // Clean up evaluated arguments
    if (own_method_name) ar_data__destroy(own_method_name);
    if (own_version) ar_data__destroy(own_version);
    // Never destroy context - the agent needs it to remain valid
    
    // Store result if assigned
    if (ar_instruction_ast__has_result_assignment(ref_ast)) {
        ar_data_t *own_result = ar_data__create_integer((int)agent_id);
        if (own_result) {
            _store_result_if_assigned(mut_memory, ref_ast, own_result);
        }
    }
    
    return success;
}
