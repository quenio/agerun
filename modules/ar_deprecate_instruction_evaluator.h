/**
 * @file ar_deprecate_instruction_evaluator.h
 * @brief Public interface for the deprecate instruction evaluator module
 */

#ifndef AGERUN_DEPRECATE_INSTRUCTION_EVALUATOR_H
#define AGERUN_DEPRECATE_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_expression_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_frame.h"
#include "ar_data.h"
#include "ar_log.h"
#include "ar_methodology.h"

/* Forward declaration of opaque struct */
typedef struct ar_deprecate_instruction_evaluator_s ar_deprecate_instruction_evaluator_t;

/**
 * Creates a new deprecate instruction evaluator instance
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param ref_expr_evaluator Expression evaluator to use (borrowed reference)
 * @param ref_methodology Methodology instance to operate on (borrowed reference)
 * @return New evaluator instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_deprecate_instruction_evaluator_t* ar_deprecate_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    ar_methodology_t *ref_methodology
);

/**
 * Destroys a deprecate instruction evaluator instance
 * @param own_evaluator The evaluator to destroy (owned value)
 * @note Ownership: Takes ownership and destroys the evaluator
 */
void ar_deprecate_instruction_evaluator__destroy(ar_deprecate_instruction_evaluator_t *own_evaluator);

/**
 * Evaluates a deprecate instruction using frame-based execution
 * @param ref_evaluator The evaluator instance (borrowed reference)
 * @param ref_frame The execution frame containing memory, context, and message (borrowed reference)
 * @param ref_ast The instruction AST to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note Ownership: Borrows all parameters, does not take ownership
 */
bool ar_deprecate_instruction_evaluator__evaluate(
    const ar_deprecate_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);



#endif /* AGERUN_DEPRECATE_INSTRUCTION_EVALUATOR_H */
