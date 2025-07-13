/**
 * @file ar_spawn_instruction_evaluator.h
 * @brief Public interface for the spawn instruction evaluator module
 *
 * This module is responsible for evaluating spawn instructions (spawn()).
 * It handles agent creation with methods and context.
 */

#ifndef AGERUN_SPAWN_INSTRUCTION_EVALUATOR_H
#define AGERUN_SPAWN_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_expression_evaluator.h"
#include "ar_data.h"
#include "ar_instruction_ast.h"
#include "ar_log.h"
#include "ar_frame.h"

/* Forward declaration of opaque struct */
typedef struct ar_spawn_instruction_evaluator_s ar_spawn_instruction_evaluator_t;

/**
 * Creates a new spawn instruction evaluator instance
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param ref_expr_evaluator The expression evaluator dependency (borrowed reference)
 * @return A newly created evaluator instance or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_spawn_instruction_evaluator_t* ar_spawn_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);

/**
 * Destroys a spawn instruction evaluator instance
 * @param own_evaluator The evaluator to destroy (owned)
 * @note Ownership: Takes ownership of the evaluator
 */
void ar_spawn_instruction_evaluator__destroy(ar_spawn_instruction_evaluator_t *own_evaluator);

/**
 * Evaluates a spawn instruction using frame-based execution
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_frame The execution frame containing memory, context, and message (borrowed reference)
 * @param ref_ast The instruction AST node
 * @return true if evaluation succeeded, false otherwise
 * @note Ownership: Does not take ownership of any parameters
 */
bool ar_spawn_instruction_evaluator__evaluate(
    ar_spawn_instruction_evaluator_t *mut_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);



#endif /* AGERUN_SPAWN_INSTRUCTION_EVALUATOR_H */
