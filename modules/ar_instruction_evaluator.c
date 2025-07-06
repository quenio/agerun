/**
 * @file agerun_instruction_evaluator.c
 * @brief Implementation of the instruction evaluator module
 */

#include "ar_instruction_evaluator.h"
#include "ar_heap.h"
#include "ar_assignment_instruction_evaluator.h"
#include "ar_send_instruction_evaluator.h"
#include "ar_condition_instruction_evaluator.h"
#include "ar_parse_instruction_evaluator.h"
#include "ar_build_instruction_evaluator.h"
#include "ar_method_instruction_evaluator.h"
#include "ar_agent_instruction_evaluator.h"
#include "ar_destroy_agent_instruction_evaluator.h"
#include "ar_destroy_method_instruction_evaluator.h"
#include "ar_list.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>


/**
 * Internal structure for instruction evaluator
 */
struct instruction_evaluator_s {
    ar_log_t *ref_log;                           /* Log instance (borrowed reference) */
    ar_expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    ar_data_t *mut_memory;                          /* Memory map (mutable reference) */
    ar_data_t *ref_context;                         /* Context map (borrowed reference, can be NULL) */
    ar_data_t *ref_message;                         /* Message data (borrowed reference, can be NULL) */
    ar_assignment_instruction_evaluator_t *own_assignment_evaluator;  /* Assignment evaluator instance (owned) */
    ar_send_instruction_evaluator_t *own_send_evaluator;  /* Send evaluator instance (owned) */
    ar_condition_instruction_evaluator_t *own_condition_evaluator;  /* Condition evaluator instance (owned) */
    ar_parse_instruction_evaluator_t *own_parse_evaluator;  /* Parse evaluator instance (owned) */
    ar_build_instruction_evaluator_t *own_build_evaluator;  /* Build evaluator instance (owned) */
    ar_method_instruction_evaluator_t *own_method_evaluator;  /* Method evaluator instance (owned) */
    ar_agent_instruction_evaluator_t *own_agent_evaluator;  /* Agent evaluator instance (owned) */
    ar_destroy_agent_instruction_evaluator_t *own_destroy_agent_evaluator;  /* Destroy agent evaluator instance (owned) */
    ar_destroy_method_instruction_evaluator_t *own_destroy_method_evaluator;  /* Destroy method evaluator instance (owned) */
};

/**
 * Creates a new instruction evaluator
 */
instruction_evaluator_t* ar_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    ar_data_t *mut_memory,
    ar_data_t *ref_context,
    ar_data_t *ref_message
) {
    // Validate required parameters
    if (ref_log == NULL || ref_expr_evaluator == NULL || mut_memory == NULL) {
        return NULL;
    }
    
    // Allocate evaluator structure
    instruction_evaluator_t *evaluator = AR__HEAP__MALLOC(sizeof(instruction_evaluator_t), "instruction_evaluator");
    if (evaluator == NULL) {
        return NULL;
    }
    
    // Initialize fields
    evaluator->ref_log = ref_log;
    evaluator->ref_expr_evaluator = ref_expr_evaluator;
    evaluator->mut_memory = mut_memory;
    evaluator->ref_context = ref_context;
    evaluator->ref_message = ref_message;
    
    // Create assignment evaluator instance
    evaluator->own_assignment_evaluator = ar_assignment_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_assignment_evaluator == NULL) {
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create send evaluator instance
    evaluator->own_send_evaluator = ar_send_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_send_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create condition evaluator instance
    evaluator->own_condition_evaluator = ar_condition_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_condition_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create parse evaluator instance
    evaluator->own_parse_evaluator = ar_parse_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_parse_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create build evaluator instance
    evaluator->own_build_evaluator = ar_build_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_build_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        ar_parse_instruction_evaluator__destroy(evaluator->own_parse_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create method evaluator instance
    evaluator->own_method_evaluator = ar_method_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_method_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        ar_parse_instruction_evaluator__destroy(evaluator->own_parse_evaluator);
        ar_build_instruction_evaluator__destroy(evaluator->own_build_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create agent evaluator instance
    evaluator->own_agent_evaluator = ar_agent_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_agent_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        ar_parse_instruction_evaluator__destroy(evaluator->own_parse_evaluator);
        ar_build_instruction_evaluator__destroy(evaluator->own_build_evaluator);
        ar_method_instruction_evaluator__destroy(evaluator->own_method_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create destroy agent evaluator instance
    evaluator->own_destroy_agent_evaluator = ar_destroy_agent_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_destroy_agent_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        ar_parse_instruction_evaluator__destroy(evaluator->own_parse_evaluator);
        ar_build_instruction_evaluator__destroy(evaluator->own_build_evaluator);
        ar_method_instruction_evaluator__destroy(evaluator->own_method_evaluator);
        ar_agent_instruction_evaluator__destroy(evaluator->own_agent_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create destroy method evaluator instance
    evaluator->own_destroy_method_evaluator = ar_destroy_method_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_destroy_method_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        ar_parse_instruction_evaluator__destroy(evaluator->own_parse_evaluator);
        ar_build_instruction_evaluator__destroy(evaluator->own_build_evaluator);
        ar_method_instruction_evaluator__destroy(evaluator->own_method_evaluator);
        ar_agent_instruction_evaluator__destroy(evaluator->own_agent_evaluator);
        ar_destroy_agent_instruction_evaluator__destroy(evaluator->own_destroy_agent_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    return evaluator;
}

/**
 * Destroys an instruction evaluator
 */
void ar_instruction_evaluator__destroy(instruction_evaluator_t *own_evaluator) {
    if (own_evaluator == NULL) {
        return;
    }
    
    // Destroy owned evaluator instances
    if (own_evaluator->own_assignment_evaluator != NULL) {
        ar_assignment_instruction_evaluator__destroy(own_evaluator->own_assignment_evaluator);
    }
    if (own_evaluator->own_send_evaluator != NULL) {
        ar_send_instruction_evaluator__destroy(own_evaluator->own_send_evaluator);
    }
    if (own_evaluator->own_condition_evaluator != NULL) {
        ar_condition_instruction_evaluator__destroy(own_evaluator->own_condition_evaluator);
    }
    if (own_evaluator->own_parse_evaluator != NULL) {
        ar_parse_instruction_evaluator__destroy(own_evaluator->own_parse_evaluator);
    }
    if (own_evaluator->own_build_evaluator != NULL) {
        ar_build_instruction_evaluator__destroy(own_evaluator->own_build_evaluator);
    }
    if (own_evaluator->own_method_evaluator != NULL) {
        ar_method_instruction_evaluator__destroy(own_evaluator->own_method_evaluator);
    }
    if (own_evaluator->own_agent_evaluator != NULL) {
        ar_agent_instruction_evaluator__destroy(own_evaluator->own_agent_evaluator);
    }
    if (own_evaluator->own_destroy_agent_evaluator != NULL) {
        ar_destroy_agent_instruction_evaluator__destroy(own_evaluator->own_destroy_agent_evaluator);
    }
    if (own_evaluator->own_destroy_method_evaluator != NULL) {
        ar_destroy_method_instruction_evaluator__destroy(own_evaluator->own_destroy_method_evaluator);
    }
    
    // Free the evaluator structure
    AR__HEAP__FREE(own_evaluator);
}



/**
 * Evaluates any instruction AST node
 */
bool ar_instruction_evaluator__evaluate(
    instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Dispatch based on AST node type
    ar_instruction_ast_type_t type = ar_instruction_ast__get_type(ref_ast);
    
    switch (type) {
        case AR_INST__ASSIGNMENT:
            // Delegate to the assignment instruction evaluator instance
            return ar_assignment_instruction_evaluator__evaluate(
                mut_evaluator->own_assignment_evaluator,
                ref_ast
            );
            
        case AR_INST__SEND:
            // Delegate to the send instruction evaluator instance
            return ar_send_instruction_evaluator__evaluate(
                mut_evaluator->own_send_evaluator,
                ref_ast
            );
            
        case AR_INST__IF:
            // Delegate to the condition instruction evaluator instance
            return ar_condition_instruction_evaluator__evaluate(
                mut_evaluator->own_condition_evaluator,
                ref_ast
            );
            
        case AR_INST__PARSE:
            // Delegate to the parse instruction evaluator instance
            return ar_parse_instruction_evaluator__evaluate(
                mut_evaluator->own_parse_evaluator,
                ref_ast
            );
            
        case AR_INST__BUILD:
            // Delegate to the build instruction evaluator instance
            return ar_build_instruction_evaluator__evaluate(
                mut_evaluator->own_build_evaluator,
                ref_ast
            );
            
        case AR_INST__METHOD:
            // Delegate to the method instruction evaluator instance
            return ar_method_instruction_evaluator__evaluate(
                mut_evaluator->own_method_evaluator,
                ref_ast
            );
            
        case AR_INST__AGENT:
            // Delegate to the agent instruction evaluator instance
            return ar_agent_instruction_evaluator__evaluate(
                mut_evaluator->own_agent_evaluator,
                mut_evaluator->ref_context,
                ref_ast
            );
            
        case AR_INST__DESTROY_AGENT:
            // Delegate directly to destroy agent evaluator
            return ar_destroy_agent_instruction_evaluator__evaluate(
                mut_evaluator->own_destroy_agent_evaluator,
                ref_ast
            );
            
        case AR_INST__DESTROY_METHOD:
            // Delegate directly to destroy method evaluator
            return ar_destroy_method_instruction_evaluator__evaluate(
                mut_evaluator->own_destroy_method_evaluator,
                ref_ast
            );
            
        default:
            // Unknown instruction type
            return false;
    }
}