#include "ar_interpreter.h"
#include "ar_heap.h"
#include "ar_agency.h"
#include "ar_agent.h"
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
    ar_frame_t *own_frame; // reusable execution frame
    ar_log_t *ref_log; // borrowed reference to log instance
    ar_agency_t *ref_agency; // borrowed reference to agency instance
};


/**
 * Creates a new interpreter instance
 */
ar_interpreter_t* ar_interpreter__create(ar_log_t *ref_log, ar_agency_t *ref_agency, ar_delegation_t *ref_delegation) {
    if (!ref_log) {
        return NULL;
    }

    ar_interpreter_t *own_interpreter = AR__HEAP__MALLOC(sizeof(ar_interpreter_t), "interpreter");
    if (!own_interpreter) {
        return NULL;
    }

    own_interpreter->ref_log = ref_log;
    own_interpreter->ref_agency = ref_agency;
    own_interpreter->own_frame = NULL;

    // Use the agency and delegation from parameters
    ar_agency_t *agency_to_use = ref_agency;
    ar_delegation_t *delegation_to_use = ref_delegation;
    if (!agency_to_use || !delegation_to_use) {
        // For backward compatibility, we need agency and delegation
        // Can't create evaluator without both
        AR__HEAP__FREE(own_interpreter);
        return NULL;
    }

    own_interpreter->own_evaluator = ar_method_evaluator__create(ref_log, agency_to_use, delegation_to_use);
    
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

    if (own_interpreter->own_frame) {
        ar_frame__destroy(own_interpreter->own_frame);
    }
    
    AR__HEAP__FREE(own_interpreter);
}


/**
 * Executes a method in the context of an agent
 */
bool ar_interpreter__execute_method(
    ar_interpreter_t *mut_interpreter,
    int64_t agent_id,
    const ar_data_t *ref_message,
    const void *ref_message_owner
) {
    if (!mut_interpreter) {
        return false;
    }
    
    // Get agent data with a single registry lookup
    const ar_method_t *ref_method = NULL;
    ar_data_t *mut_memory = NULL;
    const ar_data_t *ref_context = NULL;
    ar_agent_registry_t *ref_registry = NULL;
    ar_agent_t *ref_agent = NULL;

    if (!mut_interpreter->ref_agency) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "No agency instance available for agent %" PRId64, agent_id);
        ar_log__error(mut_interpreter->ref_log, error_msg);
        return false;
    }

    ref_registry = ar_agency__get_registry(mut_interpreter->ref_agency);
    if (!ref_registry) {
        ar_log__error(mut_interpreter->ref_log, "No agent registry available");
        return false;
    }

    ref_agent = (ar_agent_t*)ar_agent_registry__find_agent(ref_registry, agent_id);
    if (!ref_agent) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Agent %" PRId64 " not found", agent_id);
        ar_log__error(mut_interpreter->ref_log, error_msg);
        return false;
    }

    ref_method = ar_agent__get_method(ref_agent);
    mut_memory = ar_agent__get_mutable_memory(ref_agent);
    ref_context = ar_agent__get_context(ref_agent);
    
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
    
    if (!mut_interpreter->own_frame) {
        mut_interpreter->own_frame = ar_frame__create(mut_memory, ref_context, ref_message);
        if (!mut_interpreter->own_frame) {
            ar_log__error(mut_interpreter->ref_log, "Failed to create execution frame");
            return false;
        }
    } else if (!ar_frame__reset(mut_interpreter->own_frame, mut_memory, ref_context, ref_message)) {
        ar_log__error(mut_interpreter->ref_log, "Failed to reset execution frame");
        return false;
    }

    ar_frame__set_message_owner(mut_interpreter->own_frame, ref_message_owner);
    
    // Delegate to method evaluator (facade pattern)
    bool success = ar_method_evaluator__evaluate(mut_interpreter->own_evaluator, mut_interpreter->own_frame, ref_ast);
    
    if (!success) {
        ar_log__error(mut_interpreter->ref_log, "Method evaluation failed");
    }
    
    return success;
}



