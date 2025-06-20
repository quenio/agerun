/**
 * @file agerun_condition_instruction_evaluator.h
 * @brief Evaluator for if/condition instructions in AgeRun
 * 
 * This module provides functionality to evaluate conditional (if) instructions
 * that choose between two branches based on a condition expression.
 */

#ifndef AGERUN_CONDITION_INSTRUCTION_EVALUATOR_H
#define AGERUN_CONDITION_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "agerun_instruction_ast.h"
#include "agerun_expression_evaluator.h"
#include "agerun_data.h"

/* Opaque type for condition instruction evaluator */
typedef struct ar_condition_instruction_evaluator_s condition_instruction_evaluator_t;

/**
 * Creates a new condition instruction evaluator
 * 
 * @param ref_expr_evaluator The expression evaluator to use (borrowed reference)
 * @param mut_memory The memory context (mutable reference)
 * @return A new condition instruction evaluator, or NULL on failure
 * 
 * @note Ownership: Returns an owned value that caller must destroy
 */
condition_instruction_evaluator_t* ar__condition_instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
);

/**
 * Destroys a condition instruction evaluator
 * 
 * @param own_evaluator The evaluator to destroy (takes ownership)
 * 
 * @note The evaluator does not own its dependencies
 */
void ar__condition_instruction_evaluator__destroy(
    condition_instruction_evaluator_t *own_evaluator
);

/**
 * Evaluates a condition (if) instruction using stored dependencies
 * 
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The if instruction AST node to evaluate (borrowed reference)
 * @return true if the instruction was successfully evaluated, false on error
 */
bool ar__condition_instruction_evaluator__evaluate(
    condition_instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);

/**
 * Evaluates a condition (if) instruction AST node (legacy interface)
 * 
 * This function evaluates conditional instructions that have the form:
 * - result = if(condition, true_expr, false_expr)
 * - if(condition, true_expr, false_expr)
 * 
 * The condition is evaluated first. If it evaluates to a non-zero integer,
 * the true expression is evaluated. Otherwise, the false expression is evaluated.
 * Only one branch is evaluated, not both.
 * 
 * @param mut_expr_evaluator The expression evaluator to use (mutable reference)
 * @param mut_memory The memory context for storing results (mutable reference)
 * @param ref_ast The if instruction AST node to evaluate (borrowed reference)
 * @return true if the instruction was successfully evaluated, false on error
 * 
 * @note The condition must evaluate to an integer value (0 = false, non-zero = true)
 * @note Only the selected branch (true or false) is evaluated
 * @note If a result path is specified, the evaluated value is stored in memory
 * @note This is a legacy interface that will be removed once instruction_evaluator is updated
 */
bool ar_condition_instruction_evaluator__evaluate_legacy(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);

#endif /* AGERUN_CONDITION_INSTRUCTION_EVALUATOR_H */