/**
 * @file ar_compile_instruction_evaluator.h
 * @brief Public interface for the compile instruction evaluator module
 *
 * This module is responsible for evaluating compile instructions (compile()).
 * It handles method creation and registration in the methodology system.
 */

#ifndef AGERUN_COMPILE_INSTRUCTION_EVALUATOR_H
#define AGERUN_COMPILE_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_expression_evaluator.h"
#include "ar_data.h"
#include "ar_instruction_ast.h"
#include "ar_log.h"
#include "ar_frame.h"

/* Forward declaration */
typedef struct ar_methodology_s ar_methodology_t;

/**
 * Opaque type for compile instruction evaluator
 */
typedef struct ar_compile_instruction_evaluator_s ar_compile_instruction_evaluator_t;

/**
 * Creates a new compile instruction evaluator instance
 *
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param ref_expr_evaluator The expression evaluator to use (borrowed reference)
 * @param ref_methodology The methodology instance to use for method registration (borrowed reference)
 * @return A new compile instruction evaluator instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_compile_instruction_evaluator_t* ar_compile_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    ar_methodology_t *ref_methodology
);

/**
 * Destroys a compile instruction evaluator instance
 *
 * @param own_evaluator The evaluator to destroy (takes ownership)
 */
void ar_compile_instruction_evaluator__destroy(
    ar_compile_instruction_evaluator_t *own_evaluator
);

/**
 * Evaluates a compile instruction using frame-based execution
 *
 * The compile() instruction takes three string arguments:
 * - method_name: The name of the method to create
 * - instructions: The instruction code for the method
 * - version: The semantic version string (e.g., "1.0.0")
 *
 * Returns 1 on success, 0 on failure when assigned to a variable.
 *
 * @param ref_evaluator The evaluator instance to use (borrowed reference)
 * @param ref_frame The execution frame containing memory, context, and message (borrowed reference)
 * @param ref_ast The instruction AST node (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 */
bool ar_compile_instruction_evaluator__evaluate(
    const ar_compile_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);



#endif /* AGERUN_COMPILE_INSTRUCTION_EVALUATOR_H */
