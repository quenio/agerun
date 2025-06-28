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
#include "ar_frame.h"
#include "ar_instruction_ast.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>


/**
 * Internal structure for instruction evaluator
 */
struct instruction_evaluator_s {
    ar_expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    data_t *mut_memory;                          /* Memory map (mutable reference) */
    data_t *ref_context;                         /* Context map (borrowed reference, can be NULL) */
    data_t *ref_message;                         /* Message data (borrowed reference, can be NULL) */
    bool is_stateless;                           /* True if evaluator is stateless (uses frames) */
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
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory,
    data_t *ref_context,
    data_t *ref_message
) {
    // Validate required parameters
    if (ref_expr_evaluator == NULL || mut_memory == NULL) {
        return NULL;
    }
    
    // Allocate evaluator structure
    instruction_evaluator_t *evaluator = AR__HEAP__MALLOC(sizeof(instruction_evaluator_t), "instruction_evaluator");
    if (evaluator == NULL) {
        return NULL;
    }
    
    // Initialize fields
    evaluator->ref_expr_evaluator = ref_expr_evaluator;
    evaluator->mut_memory = mut_memory;
    evaluator->ref_context = ref_context;
    evaluator->ref_message = ref_message;
    evaluator->is_stateless = false;
    
    // Create assignment evaluator instance
    evaluator->own_assignment_evaluator = ar_assignment_instruction_evaluator__create(
        ref_expr_evaluator,
        mut_memory
    );
    if (evaluator->own_assignment_evaluator == NULL) {
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    // Create send evaluator instance
    evaluator->own_send_evaluator = ar_send_instruction_evaluator__create(
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
 * Creates a new stateless instruction evaluator
 */
instruction_evaluator_t* ar_instruction_evaluator__create_stateless(
    ar_expression_evaluator_t *ref_expr_evaluator
) {
    // Validate required parameters
    if (ref_expr_evaluator == NULL) {
        return NULL;
    }
    
    // Allocate evaluator structure
    instruction_evaluator_t *evaluator = AR__HEAP__MALLOC(sizeof(instruction_evaluator_t), "instruction_evaluator");
    if (evaluator == NULL) {
        return NULL;
    }
    
    // Initialize fields for stateless operation
    evaluator->ref_expr_evaluator = ref_expr_evaluator;
    evaluator->mut_memory = NULL;
    evaluator->ref_context = NULL;
    evaluator->ref_message = NULL;
    evaluator->is_stateless = true;
    
    // For stateless operation, we don't create the specialized evaluators upfront
    // They will be created on demand with the frame's memory/context
    evaluator->own_assignment_evaluator = NULL;
    evaluator->own_send_evaluator = NULL;
    evaluator->own_condition_evaluator = NULL;
    evaluator->own_parse_evaluator = NULL;
    evaluator->own_build_evaluator = NULL;
    evaluator->own_method_evaluator = NULL;
    evaluator->own_agent_evaluator = NULL;
    evaluator->own_destroy_agent_evaluator = NULL;
    evaluator->own_destroy_method_evaluator = NULL;
    
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
    ar_instruction_ast_type_t type = ar__instruction_ast__get_type(ref_ast);
    
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

/**
 * Helper to extract the key from a memory path like "memory.key"
 * Returns NULL if the path is not in the expected format
 */
static const char* _extract_memory_key(const char *path) {
    if (!path) {
        return NULL;
    }
    
    const char *dot = strchr(path, '.');
    if (!dot || strncmp(path, "memory.", 7) != 0) {
        return NULL;
    }
    
    return dot + 1;
}

/**
 * Evaluates any instruction AST node using a frame for context
 */
bool ar_instruction_evaluator__evaluate_with_frame(
    instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast,
    const ar_frame_t *ref_frame
) {
    if (!mut_evaluator || !ref_ast || !ref_frame) {
        return false;
    }
    
    // For stateless evaluation, we need to temporarily use the frame's memory/context
    // while evaluating expressions
    data_t *frame_memory = ar_frame__get_memory(ref_frame);
    // const data_t *frame_context = ar_frame__get_context(ref_frame);
    // const data_t *frame_message = ar_frame__get_message(ref_frame);
    
    // Dispatch based on AST node type
    ar_instruction_ast_type_t type = ar__instruction_ast__get_type(ref_ast);
    
    switch (type) {
        case AR_INST__ASSIGNMENT:
            {
                // For assignment, we need to evaluate the expression with the frame
                // Get the assignment details
                const char *target = ar__instruction_ast__get_assignment_path(ref_ast);
                const ar_expression_ast_t *expr_ast = ar__instruction_ast__get_assignment_expression_ast(ref_ast);
                if (!target || !expr_ast) {
                    return false;
                }
                
                // Evaluate the expression using the frame
                data_t *value = ar__expression_evaluator__evaluate_with_frame(
                    mut_evaluator->ref_expr_evaluator, expr_ast, ref_frame
                );
                if (!value) {
                    return false;
                }
                
                // Store the result in the frame's memory
                const char *key = _extract_memory_key(target);
                if (!key) {
                    ar__data__destroy(value);
                    return false;
                }
                
                bool result = ar__data__set_map_data(frame_memory, key, value);
                value = NULL; // Ownership transferred
                
                return result;
            }
            
        case AR_INST__SEND:
            {
                // For send, evaluate arguments with frame
                const list_t *arg_asts = ar__instruction_ast__get_function_arg_asts(ref_ast);
                if (!arg_asts || ar__list__count(arg_asts) != 2) {
                    return false;
                }
                
                // Get all items as array
                void **items = ar__list__items(arg_asts);
                if (!items) {
                    return false;
                }
                
                // Evaluate target
                ar_expression_ast_t *target_ast = (ar_expression_ast_t *)items[0];
                data_t *target = ar__expression_evaluator__evaluate_with_frame(
                    mut_evaluator->ref_expr_evaluator, target_ast, ref_frame
                );
                if (!target || ar__data__get_type(target) != DATA_INTEGER) {
                    if (target) ar__data__destroy(target);
                    AR__HEAP__FREE(items);
                    return false;
                }
                
                // Evaluate message
                ar_expression_ast_t *msg_ast = (ar_expression_ast_t *)items[1];
                data_t *message = ar__expression_evaluator__evaluate_with_frame(
                    mut_evaluator->ref_expr_evaluator, msg_ast, ref_frame
                );
                if (!message) {
                    ar__data__destroy(target);
                    AR__HEAP__FREE(items);
                    return false;
                }
                
                AR__HEAP__FREE(items);
                
                // For now, just validate and clean up (actual send would happen in specialized evaluator)
                bool result = (ar__data__get_integer(target) == 0); // send to 0 is valid
                
                ar__data__destroy(target);
                ar__data__destroy(message);
                
                return result;
            }
            
        case AR_INST__IF:
            {
                // For if, evaluate condition and appropriate branch with frame
                const list_t *arg_asts = ar__instruction_ast__get_function_arg_asts(ref_ast);
                if (!arg_asts || ar__list__count(arg_asts) != 3) {
                    return false;
                }
                
                // Get all items as array
                void **items = ar__list__items(arg_asts);
                if (!items) {
                    return false;
                }
                
                // Evaluate condition
                ar_expression_ast_t *cond_ast = (ar_expression_ast_t *)items[0];
                data_t *condition = ar__expression_evaluator__evaluate_with_frame(
                    mut_evaluator->ref_expr_evaluator, cond_ast, ref_frame
                );
                if (!condition) {
                    AR__HEAP__FREE(items);
                    return false;
                }
                
                // Determine which branch to evaluate
                bool is_true = false;
                if (ar__data__get_type(condition) == DATA_INTEGER) {
                    is_true = (ar__data__get_integer(condition) != 0);
                }
                ar__data__destroy(condition);
                
                // Evaluate the appropriate branch
                ar_expression_ast_t *branch_ast = (ar_expression_ast_t *)items[is_true ? 1 : 2];
                data_t *result = ar__expression_evaluator__evaluate_with_frame(
                    mut_evaluator->ref_expr_evaluator, branch_ast, ref_frame
                );
                
                AR__HEAP__FREE(items);
                
                if (!result) {
                    return false;
                }
                
                // Store result if there's a target
                const char *target = ar__instruction_ast__get_function_result_path(ref_ast);
                if (target) {
                    const char *key = _extract_memory_key(target);
                    if (key) {
                        ar__data__set_map_data(frame_memory, key, result);
                        result = NULL; // Ownership transferred
                    }
                }
                
                if (result) {
                    ar__data__destroy(result);
                }
                
                return true;
            }
            
        default:
            // For other instruction types, delegate to specialized evaluators
            // which would need to be updated to support frames
            return false;
    }
}