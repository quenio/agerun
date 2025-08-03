#include "ar_interpreter.h"
#include "ar_heap.h"
#include "ar_agency.h"
#include "ar_data.h"
#include "ar_method_evaluator.h"
#include "ar_frame.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

/**
 * Interpreter structure (private implementation)
 */
struct ar_interpreter_s {
    ar_method_evaluator_t *own_evaluator; // owned method evaluator
    ar_log_t *ref_log; // borrowed reference to log instance
    ar_agency_t *ref_agency; // borrowed reference to agency instance
};


/**
 * Creates a new interpreter instance
 */
ar_interpreter_t* ar_interpreter__create(ar_log_t *ref_log) {
    // Keep old signature for backward compatibility
    return ar_interpreter__create_with_agency(ref_log, NULL);
}

ar_interpreter_t* ar_interpreter__create_with_agency(ar_log_t *ref_log, ar_agency_t *ref_agency) {
    if (!ref_log) {
        return NULL;
    }
    
    ar_interpreter_t *own_interpreter = AR__HEAP__MALLOC(sizeof(ar_interpreter_t), "interpreter");
    if (!own_interpreter) {
        return NULL;
    }
    
    own_interpreter->ref_log = ref_log;
    own_interpreter->ref_agency = ref_agency;
    
    // Use the agency from parameter, or get the global one for backward compatibility
    ar_agency_t *agency_to_use = ref_agency;
    if (!agency_to_use) {
        // For backward compatibility, we need a way to get the global agency
        // This will be NULL if no global system exists
        return NULL; // Can't create evaluator without agency
    }
    
    own_interpreter->own_evaluator = ar_method_evaluator__create(ref_log, agency_to_use);
    
    if (!own_interpreter->own_evaluator) {
        AR__HEAP__FREE(own_interpreter);
        return NULL;
    }
    
    return own_interpreter;
    // Ownership transferred to caller
}

/**
 * Destroys an interpreter instance and frees its resources
 */
void ar_interpreter__destroy(ar_interpreter_t *own_interpreter) {
    if (!own_interpreter) {
        return;
    }
    
    if (own_interpreter->own_evaluator) {
        ar_method_evaluator__destroy(own_interpreter->own_evaluator);
    }
    
    AR__HEAP__FREE(own_interpreter);
}


/**
 * Executes a method in the context of an agent
 */
bool ar_interpreter__execute_method(ar_interpreter_t *mut_interpreter,
                                    int64_t agent_id, 
                                    const ar_data_t *ref_message) {
    if (!mut_interpreter) {
        return false;
    }
    
    // DEBUG: Log method execution
    if (ref_message) {
        fprintf(stderr, "DEBUG [INTERPRETER]: Executing method for agent %lld with message type=%u\n", 
               (long long)agent_id, 
               (unsigned int)ar_data__get_type(ref_message));
    } else {
        fprintf(stderr, "DEBUG [INTERPRETER]: Executing method for agent %lld with NULL message\n", 
               (long long)agent_id);
    }
    
    // Get agent method - use instance agency if available
    const ar_method_t *ref_method = NULL;
    ar_data_t *mut_memory = NULL;
    const ar_data_t *ref_context = NULL;
    
    if (mut_interpreter->ref_agency) {
        // Use instance-based agency functions
        ref_method = ar_agency__get_agent_method_with_instance(mut_interpreter->ref_agency, agent_id);
        mut_memory = ar_agency__get_agent_mutable_memory_with_instance(mut_interpreter->ref_agency, agent_id);
        ref_context = ar_agency__get_agent_context_with_instance(mut_interpreter->ref_agency, agent_id);
    } else {
        // Use global agency functions
        ref_method = ar_agency__get_agent_method(agent_id);
        mut_memory = ar_agency__get_agent_mutable_memory(agent_id);
        ref_context = ar_agency__get_agent_context(agent_id);
    }
    
    if (!ref_method) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Agent %" PRId64 " has no method", agent_id);
        ar_log__error(mut_interpreter->ref_log, error_msg);
        return false;
    }
    
    if (!mut_memory) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Agent %" PRId64 " has no memory", agent_id);
        ar_log__error(mut_interpreter->ref_log, error_msg);
        return false;
    }
    
    // Get the method AST
    const ar_method_ast_t *ref_ast = ar_method__get_ast(ref_method);
    if (!ref_ast) {
        ar_log__error(mut_interpreter->ref_log, "Method has no AST");
        return false;
    }
    
    // Create a frame for execution
    ar_frame_t *own_frame = ar_frame__create(mut_memory, ref_context, ref_message);
    if (!own_frame) {
        ar_log__error(mut_interpreter->ref_log, "Failed to create execution frame");
        return false;
    }
    
    // Delegate to method evaluator (facade pattern)
    bool success = ar_method_evaluator__evaluate(mut_interpreter->own_evaluator, own_frame, ref_ast);
    
    if (!success) {
        ar_log__error(mut_interpreter->ref_log, "Method evaluation failed");
    }
    
    // Clean up frame
    ar_frame__destroy(own_frame);
    
    return success;
}



