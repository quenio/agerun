/**
 * @file agerun_destroy_instruction_evaluator.h
 * @brief Destroy instruction evaluator module
 * 
 * This module is responsible for evaluating destroy instructions in the AgeRun language.
 * It handles both agent destruction (1 argument) and method destruction (2 arguments).
 */

#ifndef AGERUN_DESTROY_INSTRUCTION_EVALUATOR_H
#define AGERUN_DESTROY_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_data.h"

/**
 * Evaluates a destroy instruction
 * @param mut_expr_evaluator Expression evaluator to use (mutable reference)
 * @param mut_memory Memory map to use (mutable reference)
 * @param ref_ast Instruction AST to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note The function handles both destroy(agent_id) and destroy(method_name, method_version)
 */
bool ar__destroy_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);

#endif // AGERUN_DESTROY_INSTRUCTION_EVALUATOR_H