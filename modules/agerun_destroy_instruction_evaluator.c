/**
 * @file agerun_destroy_instruction_evaluator.c
 * @brief Implementation of the destroy instruction evaluator module
 */

#include "agerun_destroy_instruction_evaluator.h"
#include "agerun_destroy_agent_instruction_evaluator.h"
#include "agerun_destroy_method_instruction_evaluator.h"
#include "agerun_heap.h"
#include "agerun_list.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>


bool ar__destroy_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
) {
    if (!mut_expr_evaluator || !mut_memory || !ref_ast) {
        return false;
    }
    
    // Validate AST type
    if (ar__instruction_ast__get_type(ref_ast) != INST_AST_DESTROY) {
        return false;
    }
    
    // Get function arguments to determine which evaluator to use
    list_t *own_args = ar__instruction_ast__get_function_args(ref_ast);
    if (!own_args) {
        return false;
    }
    
    size_t arg_count = ar__list__count(own_args);
    ar__list__destroy(own_args);
    
    if (arg_count == 1) {
        // destroy(agent_id) - dispatch to destroy agent evaluator
        return ar__destroy_agent_instruction_evaluator__evaluate_legacy(
            mut_expr_evaluator,
            mut_memory,
            ref_ast
        );
    } else if (arg_count == 2) {
        // destroy(method_name, method_version) - dispatch to destroy method evaluator
        return ar__destroy_method_instruction_evaluator__evaluate_legacy(
            mut_expr_evaluator,
            mut_memory,
            ref_ast
        );
    } else {
        // Invalid argument count
        return false;
    }
}