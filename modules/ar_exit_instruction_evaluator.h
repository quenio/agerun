/**
 * @file ar_exit_instruction_evaluator.h
 * @brief Public interface for the exit instruction evaluator module
 */

#ifndef AGERUN_EXIT_INSTRUCTION_EVALUATOR_H
#define AGERUN_EXIT_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_expression_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_data.h"
#include "ar_log.h"
#include "ar_frame.h"

/* Forward declaration of opaque struct */
typedef struct ar_exit_instruction_evaluator_s ar_exit_instruction_evaluator_t;

/**
 * Creates a new exit agent instruction evaluator instance
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param ref_expr_evaluator Expression evaluator to use (borrowed reference)
 * @return New evaluator instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_exit_instruction_evaluator_t* ar_exit_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);

/**
 * Destroys a exit agent instruction evaluator instance
 * @param own_evaluator The evaluator to destroy (owned value)
 * @note Ownership: Takes ownership and destroys the evaluator
 */
void ar_exit_instruction_evaluator__destroy(ar_exit_instruction_evaluator_t *own_evaluator);

/**
 * Evaluates a exit agent instruction using frame-based execution
 * @param ref_evaluator The evaluator instance (borrowed reference)
 * @param ref_frame The execution frame containing memory, context, and message (borrowed reference)
 * @param ref_ast The instruction AST to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note Ownership: Borrows all parameters, does not take ownership
 */
bool ar_exit_instruction_evaluator__evaluate(
    const ar_exit_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);



#endif /* AGERUN_EXIT_INSTRUCTION_EVALUATOR_H */
