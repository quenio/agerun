/**
 * @file ar_method_instruction_evaluator.h
 * @brief Public interface for the method instruction evaluator module
 *
 * This module is responsible for evaluating method instructions (method()).
 * It handles method creation and registration in the methodology system.
 */

#ifndef AGERUN_METHOD_INSTRUCTION_EVALUATOR_H
#define AGERUN_METHOD_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_expression_evaluator.h"
#include "ar_data.h"
#include "ar_instruction_ast.h"
#include "ar_log.h"
#include "ar_frame.h"

/**
 * Opaque type for method instruction evaluator
 */
typedef struct ar_method_instruction_evaluator_s ar_method_instruction_evaluator_t;

/**
 * Creates a new method instruction evaluator instance
 *
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param ref_expr_evaluator The expression evaluator to use (borrowed reference)
 * @return A new method instruction evaluator instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_method_instruction_evaluator_t* ar_method_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);

/**
 * Destroys a method instruction evaluator instance
 *
 * @param own_evaluator The evaluator to destroy (takes ownership)
 */
void ar_method_instruction_evaluator__destroy(
    ar_method_instruction_evaluator_t *own_evaluator
);

/**
 * Evaluates a method instruction using frame-based execution
 *
 * The method() instruction takes three string arguments:
 * - method_name: The name of the method to create
 * - instructions: The instruction code for the method
 * - version: The semantic version string (e.g., "1.0.0")
 *
 * Returns 1 on success, 0 on failure when assigned to a variable.
 *
 * @param mut_evaluator The evaluator instance to use
 * @param ref_frame The execution frame containing memory, context, and message (borrowed reference)
 * @param ref_ast The instruction AST node
 * @return true if evaluation succeeded, false otherwise
 */
bool ar_method_instruction_evaluator__evaluate(
    ar_method_instruction_evaluator_t *mut_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);



#endif /* AGERUN_METHOD_INSTRUCTION_EVALUATOR_H */
