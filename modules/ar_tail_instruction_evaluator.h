/**
 * @file ar_tail_instruction_evaluator.h
 * @brief Tail instruction evaluator module for AgeRun
 */

#ifndef AGERUN_TAIL_INSTRUCTION_EVALUATOR_H
#define AGERUN_TAIL_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_expression_evaluator.h"
#include "ar_frame.h"
#include "ar_instruction_ast.h"
#include "ar_log.h"

/**
 * Opaque type for tail instruction evaluator.
 */
typedef struct ar_tail_instruction_evaluator_s ar_tail_instruction_evaluator_t;

/**
 * Creates a new tail instruction evaluator.
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param ref_expr_evaluator The expression evaluator to use (borrowed reference)
 * @return A new tail instruction evaluator, or NULL on error
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_tail_instruction_evaluator_t* ar_tail_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);

/**
 * Destroys a tail instruction evaluator.
 * @param own_evaluator The evaluator to destroy (takes ownership)
 */
void ar_tail_instruction_evaluator__destroy(
    ar_tail_instruction_evaluator_t *own_evaluator
);

/**
 * Evaluates a tail instruction AST node using frame-based execution.
 * @param ref_evaluator The tail instruction evaluator to use (borrowed reference)
 * @param ref_frame The execution frame containing memory, context, and message (borrowed reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 */
bool ar_tail_instruction_evaluator__evaluate(
    const ar_tail_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);

#endif /* AGERUN_TAIL_INSTRUCTION_EVALUATOR_H */
