/**
 * @file agerun_assignment_instruction_evaluator.h
 * @brief Assignment instruction evaluator module for AgeRun
 * 
 * This module provides functionality to evaluate assignment instruction AST nodes.
 * It handles variable assignments in the form "memory.variable := expression".
 */

#ifndef AGERUN_ASSIGNMENT_INSTRUCTION_EVALUATOR_H
#define AGERUN_ASSIGNMENT_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "agerun_instruction_ast.h"
#include "agerun_data.h"
#include "agerun_expression_evaluator.h"

/**
 * Evaluates an assignment instruction AST node
 * @param mut_expr_evaluator The expression evaluator to use (mutable reference)
 * @param mut_memory The memory map to modify (mutable reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note The assignment will modify the memory map by storing the evaluated expression result
 */
bool ar__assignment_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);

#endif /* AGERUN_ASSIGNMENT_INSTRUCTION_EVALUATOR_H */