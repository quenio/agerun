/**
 * @file ar_destroy_agent_instruction_evaluator.h
 * @brief Public interface for the destroy agent instruction evaluator module
 */

#ifndef AGERUN_DESTROY_AGENT_INSTRUCTION_EVALUATOR_H
#define AGERUN_DESTROY_AGENT_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_expression_evaluator.h"
#include "ar_instruction_ast.h"
#include "ar_data.h"
#include "ar_log.h"

/* Forward declaration of opaque struct */
typedef struct ar_destroy_agent_instruction_evaluator_s ar_destroy_agent_instruction_evaluator_t;

/**
 * Creates a new destroy agent instruction evaluator instance
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param mut_expr_evaluator Expression evaluator to use (mutable reference)
 * @param mut_memory Memory data structure (mutable reference)
 * @return New evaluator instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_destroy_agent_instruction_evaluator_t* ar_destroy_agent_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *mut_expr_evaluator,
    ar_data_t *mut_memory
);

/**
 * Destroys a destroy agent instruction evaluator instance
 * @param own_evaluator The evaluator to destroy (owned value)
 * @note Ownership: Takes ownership and destroys the evaluator
 */
void ar_destroy_agent_instruction_evaluator__destroy(ar_destroy_agent_instruction_evaluator_t *own_evaluator);

/**
 * Evaluates a destroy agent instruction using stored dependencies
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The instruction AST to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note Ownership: Borrows all parameters, does not take ownership
 */
bool ar_destroy_agent_instruction_evaluator__evaluate(
    ar_destroy_agent_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
);



#endif /* AGERUN_DESTROY_AGENT_INSTRUCTION_EVALUATOR_H */
