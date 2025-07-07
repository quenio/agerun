/**
 * @file agerun_destroy_method_instruction_evaluator.c
 * @brief Implementation of the destroy method instruction evaluator module
 */

#include "ar_destroy_method_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_expression_ast.h"
#include "ar_expression_evaluator.h"
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
struct ar_destroy_method_instruction_evaluator_s {
    ar_log_t *ref_log;                           /* Borrowed reference to log instance */
    ar_expression_evaluator_t *mut_expr_evaluator;
    ar_data_t *mut_memory;
};

/* Helper function to log error message */
static void _log_error(ar_destroy_method_instruction_evaluator_t *mut_evaluator, const char *message) {
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
 * Creates a new destroy method instruction evaluator instance
 */
ar_destroy_method_instruction_evaluator_t* ar_destroy_method_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *mut_expr_evaluator,
    ar_data_t *mut_memory
) {
    if (!ref_log || !mut_expr_evaluator || !mut_memory) {
        return NULL;
    }
    
    ar_destroy_method_instruction_evaluator_t *own_evaluator = AR__HEAP__MALLOC(
        sizeof(ar_destroy_method_instruction_evaluator_t), 
        "destroy_method_instruction_evaluator"
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
 * Destroys a destroy method instruction evaluator instance
 */
void ar_destroy_method_instruction_evaluator__destroy(ar_destroy_method_instruction_evaluator_t *own_evaluator) {
    if (!own_evaluator) {
        return;
    }
    
    AR__HEAP__FREE(own_evaluator);
}

/**
 * Evaluates a destroy method instruction using stored dependencies
 */
bool ar_destroy_method_instruction_evaluator__evaluate(
    ar_destroy_method_instruction_evaluator_t *mut_evaluator,
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
    if (ar_instruction_ast__get_type(ref_ast) != AR_INSTRUCTION_AST_TYPE__DESTROY_METHOD) {
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
    ar_data_t *name_result = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_name_ast);
    ar_data_t *version_result = ar_expression_evaluator__evaluate(mut_expr_evaluator, ref_version_ast);
    
    // Handle ownership for name
    ar_data_t *own_name = NULL;
    if (name_result) {
        if (ar_data__hold_ownership(name_result, mut_expr_evaluator)) {
            // We can claim ownership - it's an unowned value
            ar_data__transfer_ownership(name_result, mut_expr_evaluator);
            own_name = name_result;
        } else {
            // It's owned by someone else - we need to make a copy
            own_name = ar_data__shallow_copy(name_result);
            if (!own_name) {
                _log_error(mut_evaluator, "Cannot destroy method with nested containers in name (no deep copy support)");
                AR__HEAP__FREE(items);
                return false;
            }
        }
    }
    
    // Handle ownership for version
    ar_data_t *own_version = NULL;
    if (version_result) {
        if (ar_data__hold_ownership(version_result, mut_expr_evaluator)) {
            // We can claim ownership - it's an unowned value
            ar_data__transfer_ownership(version_result, mut_expr_evaluator);
            own_version = version_result;
        } else {
            // It's owned by someone else - we need to make a copy
            own_version = ar_data__shallow_copy(version_result);
            if (!own_version) {
                _log_error(mut_evaluator, "Cannot destroy method with nested containers in version (no deep copy support)");
                if (own_name) ar_data__destroy(own_name);
                AR__HEAP__FREE(items);
                return false;
            }
        }
    }
    
    bool success = false;
    bool destroy_result = false;
    
    if (own_name && own_version &&
        ar_data__get_type(own_name) == AR_DATA_TYPE__STRING &&
        ar_data__get_type(own_version) == AR_DATA_TYPE__STRING) {
        
        const char *method_name = ar_data__get_string(own_name);
        const char *method_version = ar_data__get_string(own_version);
        
        // Get the method to check if agents are using it
        ar_method_t *ref_method = ar_methodology__get_method(method_name, method_version);
        if (ref_method) {
            // Count agents using this method
            int agent_count = ar_agency__count_agents_using_method(ref_method);
            
            if (agent_count > 0) {
                // Send __sleep__ messages to all agents using this method
                int64_t agent_id = ar_agency__get_first_agent();
                while (agent_id > 0) {
                    const ar_method_t *agent_method = ar_agency__get_agent_method(agent_id);
                    if (agent_method == ref_method) {
                        ar_data_t *sleep_msg = ar_data__create_string("__sleep__");
                        if (sleep_msg) {
                            bool sent = ar_agency__send_to_agent(agent_id, sleep_msg);
                            if (!sent) {
                                // If send fails, we need to destroy the message ourselves
                                ar_data__destroy(sleep_msg);
                            }
                        }
                    }
                    agent_id = ar_agency__get_next_agent(agent_id);
                }
                
                // Now destroy each agent
                agent_id = ar_agency__get_first_agent();
                while (agent_id > 0) {
                    int64_t next_id = ar_agency__get_next_agent(agent_id);
                    const ar_method_t *agent_method = ar_agency__get_agent_method(agent_id);
                    if (agent_method == ref_method) {
                        ar_agency__destroy_agent(agent_id);
                    }
                    agent_id = next_id;
                }
            }
            
            // Now unregister the method
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
            _store_result_if_assigned(mut_memory, ref_ast, own_result);
        }
    }
    
    return success;
}
