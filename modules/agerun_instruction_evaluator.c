/**
 * @file agerun_instruction_evaluator.c
 * @brief Implementation of the instruction evaluator module
 */

#include "agerun_instruction_evaluator.h"
#include "agerun_heap.h"
#include "agerun_expression_parser.h"
#include "agerun_expression_ast.h"
#include "agerun_agency.h"
#include "agerun_method.h"
#include "agerun_methodology.h"
#include "agerun_assignment_instruction_evaluator.h"
#include "agerun_send_instruction_evaluator.h"
#include "agerun_condition_instruction_evaluator.h"
#include "agerun_parse_instruction_evaluator.h"
#include "agerun_build_instruction_evaluator.h"
#include "agerun_method_instruction_evaluator.h"
#include "agerun_agent_instruction_evaluator.h"
#include "agerun_destroy_instruction_evaluator.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>


/**
 * Internal structure for instruction evaluator
 */
struct instruction_evaluator_s {
    expression_evaluator_t *ref_expr_evaluator;  /* Expression evaluator (borrowed reference) */
    data_t *mut_memory;                          /* Memory map (mutable reference) */
    data_t *ref_context;                         /* Context map (borrowed reference, can be NULL) */
    data_t *ref_message;                         /* Message data (borrowed reference, can be NULL) */
};

/**
 * Creates a new instruction evaluator
 */
instruction_evaluator_t* ar__instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
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
    
    return evaluator;
}

/**
 * Destroys an instruction evaluator
 */
void ar__instruction_evaluator__destroy(instruction_evaluator_t *own_evaluator) {
    if (own_evaluator == NULL) {
        return;
    }
    
    // Free the evaluator structure
    AR__HEAP__FREE(own_evaluator);
}


bool ar__instruction_evaluator__evaluate_assignment(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the assignment instruction evaluator module (using legacy interface for now)
    return ar_assignment_instruction_evaluator__evaluate_legacy(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        ref_ast
    );
}

bool ar__instruction_evaluator__evaluate_send(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the send instruction evaluator module (using legacy interface for now)
    return ar_send_instruction_evaluator__evaluate_legacy(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        ref_ast
    );
}

bool ar__instruction_evaluator__evaluate_if(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the condition instruction evaluator module
    return ar_condition_instruction_evaluator__evaluate_legacy(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        ref_ast
    );
}


bool ar__instruction_evaluator__evaluate_parse(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the parse instruction evaluator module (using legacy interface for now)
    return ar_parse_instruction_evaluator__evaluate_legacy(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        ref_ast
    );
}


bool ar__instruction_evaluator__evaluate_build(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the build instruction evaluator module
    return ar_build_instruction_evaluator__evaluate(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        ref_ast
    );
}

bool ar__instruction_evaluator__evaluate_method(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the method instruction evaluator module
    return ar_method_instruction_evaluator__evaluate(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        ref_ast
    );
}

bool ar__instruction_evaluator__evaluate_agent(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the agent instruction evaluator module
    return ar__agent_instruction_evaluator__evaluate(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        mut_evaluator->ref_context,
        ref_ast
    );
}

bool ar__instruction_evaluator__evaluate_destroy(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
) {
    if (!mut_evaluator || !ref_ast) {
        return false;
    }
    
    // Delegate to the destroy instruction evaluator module
    return ar__destroy_instruction_evaluator__evaluate(
        mut_evaluator->ref_expr_evaluator,
        mut_evaluator->mut_memory,
        ref_ast
    );
}