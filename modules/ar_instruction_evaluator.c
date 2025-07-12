/**
 * @file ar_instruction_evaluator.c
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
#include "ar_frame.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>


/**
 * Internal structure for instruction evaluator
 */
struct ar_instruction_evaluator_s {
    ar_log_t *ref_log;                           /* Log instance (borrowed reference) */
    ar_expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
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
ar_instruction_evaluator_t* ar_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
) {
    // Validate required parameters
    if (ref_log == NULL || ref_expr_evaluator == NULL) {
        return NULL;
    }
    
    // Allocate evaluator structure
    ar_instruction_evaluator_t *evaluator = AR__HEAP__MALLOC(sizeof(ar_instruction_evaluator_t), "instruction_evaluator");
    if (evaluator == NULL) {
        return NULL;
    }
    
    // Initialize fields
    evaluator->ref_log = ref_log;
    evaluator->ref_expr_evaluator = ref_expr_evaluator;
    
    // Create assignment evaluator instance (now uses frame-based pattern)
    evaluator->own_assignment_evaluator = ar_assignment_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator
    );
    if (evaluator->own_assignment_evaluator == NULL) {
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create send evaluator instance (now uses frame-based pattern)
    evaluator->own_send_evaluator = ar_send_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator
    );
    if (evaluator->own_send_evaluator == NULL) {
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create condition evaluator instance (now uses frame-based pattern)
    evaluator->own_condition_evaluator = ar_condition_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator
    );
    if (evaluator->own_condition_evaluator == NULL) {
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create parse evaluator instance (now uses frame-based pattern)
    evaluator->own_parse_evaluator = ar_parse_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator
    );
    if (evaluator->own_parse_evaluator == NULL) {
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create build evaluator instance (now uses frame-based pattern)
    evaluator->own_build_evaluator = ar_build_instruction_evaluator__create(
        ref_log,
        ref_expr_evaluator
    );
    if (evaluator->own_build_evaluator == NULL) {
        ar_parse_instruction_evaluator__destroy(evaluator->own_parse_evaluator);
        ar_condition_instruction_evaluator__destroy(evaluator->own_condition_evaluator);
        ar_send_instruction_evaluator__destroy(evaluator->own_send_evaluator);
        ar_assignment_instruction_evaluator__destroy(evaluator->own_assignment_evaluator);
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    evaluator->own_method_evaluator = NULL;
    evaluator->own_agent_evaluator = NULL;
    evaluator->own_destroy_agent_evaluator = NULL;
    evaluator->own_destroy_method_evaluator = NULL;
    
    return evaluator;
}

/**
 * Destroys an instruction evaluator
 */
void ar_instruction_evaluator__destroy(ar_instruction_evaluator_t *own_evaluator) {
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
    ar_instruction_evaluator_t *mut_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_frame || !ref_ast) {
        return false;
    }
    
    // Dispatch based on AST node type
    ar_instruction_ast_type_t type = ar_instruction_ast__get_type(ref_ast);
    
    switch (type) {
        case AR_INSTRUCTION_AST_TYPE__ASSIGNMENT:
            // Delegate to the assignment instruction evaluator instance (with frame)
            return ar_assignment_instruction_evaluator__evaluate(
                mut_evaluator->own_assignment_evaluator,
                ref_frame,
                ref_ast
            );
            
        case AR_INSTRUCTION_AST_TYPE__SEND:
            // Delegate to the send instruction evaluator instance
            return ar_send_instruction_evaluator__evaluate(
                mut_evaluator->own_send_evaluator,
                ref_frame,
                ref_ast
            );
            
        case AR_INSTRUCTION_AST_TYPE__IF:
            // Delegate to the condition instruction evaluator instance (with frame)
            return ar_condition_instruction_evaluator__evaluate(
                mut_evaluator->own_condition_evaluator,
                ref_frame,
                ref_ast
            );
            
        case AR_INSTRUCTION_AST_TYPE__PARSE:
            // Delegate to the parse instruction evaluator instance
            return ar_parse_instruction_evaluator__evaluate(
                mut_evaluator->own_parse_evaluator,
                ref_frame,
                ref_ast
            );
            
        case AR_INSTRUCTION_AST_TYPE__BUILD:
            // Delegate to the build instruction evaluator instance (with frame)
            return ar_build_instruction_evaluator__evaluate(
                mut_evaluator->own_build_evaluator,
                ref_frame,
                ref_ast
            );
            
        case AR_INSTRUCTION_AST_TYPE__METHOD:
            // Create method evaluator on-demand if needed
            if (mut_evaluator->own_method_evaluator == NULL) {
                ar_data_t *memory = ar_frame__get_memory(ref_frame);
                mut_evaluator->own_method_evaluator = ar_method_instruction_evaluator__create(
                    mut_evaluator->ref_log,
                    mut_evaluator->ref_expr_evaluator,
                    memory
                );
                if (mut_evaluator->own_method_evaluator == NULL) {
                    return false;
                }
            }
            // Delegate to the method instruction evaluator instance
            return ar_method_instruction_evaluator__evaluate(
                mut_evaluator->own_method_evaluator,
                ref_ast
            );
            
        case AR_INSTRUCTION_AST_TYPE__AGENT:
            // Create agent evaluator on-demand if needed
            if (mut_evaluator->own_agent_evaluator == NULL) {
                ar_data_t *memory = ar_frame__get_memory(ref_frame);
                mut_evaluator->own_agent_evaluator = ar_agent_instruction_evaluator__create(
                    mut_evaluator->ref_log,
                    mut_evaluator->ref_expr_evaluator,
                    memory
                );
                if (mut_evaluator->own_agent_evaluator == NULL) {
                    return false;
                }
            }
            // Delegate to the agent instruction evaluator instance
            const ar_data_t *context = ar_frame__get_context(ref_frame);
            return ar_agent_instruction_evaluator__evaluate(
                mut_evaluator->own_agent_evaluator,
                context,
                ref_ast
            );
            
        case AR_INSTRUCTION_AST_TYPE__DESTROY_AGENT:
            // Create destroy agent evaluator on-demand if needed
            if (mut_evaluator->own_destroy_agent_evaluator == NULL) {
                ar_data_t *memory = ar_frame__get_memory(ref_frame);
                mut_evaluator->own_destroy_agent_evaluator = ar_destroy_agent_instruction_evaluator__create(
                    mut_evaluator->ref_log,
                    mut_evaluator->ref_expr_evaluator,
                    memory
                );
                if (mut_evaluator->own_destroy_agent_evaluator == NULL) {
                    return false;
                }
            }
            // Delegate directly to destroy agent evaluator
            return ar_destroy_agent_instruction_evaluator__evaluate(
                mut_evaluator->own_destroy_agent_evaluator,
                ref_ast
            );
            
        case AR_INSTRUCTION_AST_TYPE__DESTROY_METHOD:
            // Create destroy method evaluator on-demand if needed
            if (mut_evaluator->own_destroy_method_evaluator == NULL) {
                ar_data_t *memory = ar_frame__get_memory(ref_frame);
                mut_evaluator->own_destroy_method_evaluator = ar_destroy_method_instruction_evaluator__create(
                    mut_evaluator->ref_log,
                    mut_evaluator->ref_expr_evaluator,
                    memory
                );
                if (mut_evaluator->own_destroy_method_evaluator == NULL) {
                    return false;
                }
            }
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