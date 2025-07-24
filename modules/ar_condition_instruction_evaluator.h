/**
 * @file ar_condition_instruction_evaluator.h
 * @brief Evaluator for if/condition instructions in AgeRun
 *
 * This module provides functionality to evaluate conditional (if) instructions
 * that choose between two branches based on a condition expression.
 */

#ifndef AGERUN_CONDITION_INSTRUCTION_EVALUATOR_H
#define AGERUN_CONDITION_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_instruction_ast.h"
#include "ar_expression_evaluator.h"
#include "ar_data.h"
#include "ar_log.h"
#include "ar_frame.h"

/* Opaque type for condition instruction evaluator */
typedef struct ar_condition_instruction_evaluator_s ar_condition_instruction_evaluator_t;

/**
 * Creates a new condition instruction evaluator
 *
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param ref_expr_evaluator The expression evaluator to use (borrowed reference)
 * @return A new condition instruction evaluator, or NULL on failure
 *
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_condition_instruction_evaluator_t* ar_condition_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);

/**
 * Destroys a condition instruction evaluator
 *
 * @param own_evaluator The evaluator to destroy (takes ownership)
 *
 * @note The evaluator does not own its dependencies
 */
void ar_condition_instruction_evaluator__destroy(
    ar_condition_instruction_evaluator_t *own_evaluator
);

/**
 * Evaluates a condition (if) instruction using frame-based execution
 *
 * @param ref_evaluator The evaluator instance (const reference)
 * @param ref_frame The frame containing memory and context (borrowed reference)
 * @param ref_ast The if instruction AST node to evaluate (borrowed reference)
 * @return true if the instruction was successfully evaluated, false on error
 */
bool ar_condition_instruction_evaluator__evaluate(
    const ar_condition_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);


/**
 * Evaluates a condition (if) instruction (legacy interface)
 * @param mut_expr_evaluator The expression evaluator
 * @param mut_memory The memory map
 * @param ref_ast The instruction AST node
 * @return true if evaluation succeeded, false otherwise
 * @note Ownership: Does not take ownership of any parameters
 */
bool ar_condition_instruction_evaluator__evaluate_legacy(
    ar_expression_evaluator_t *mut_expr_evaluator,
    ar_data_t *mut_memory,
    const ar_instruction_ast_t *ref_ast
);

#endif /* AGERUN_CONDITION_INSTRUCTION_EVALUATOR_H */
