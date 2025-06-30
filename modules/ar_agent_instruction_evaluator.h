/**
 * @file agerun_agent_instruction_evaluator.h
 * @brief Public interface for the agent instruction evaluator module
 *
 * This module is responsible for evaluating agent instructions (agent()).
 * It handles agent creation with methods and context.
 */

#ifndef AGERUN_AGENT_INSTRUCTION_EVALUATOR_H
#define AGERUN_AGENT_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_expression_evaluator.h"
#include "ar_data.h"
#include "ar_instruction_ast.h"
#include "ar_log.h"

/* Forward declaration of opaque struct */
typedef struct ar_agent_instruction_evaluator_s ar_agent_instruction_evaluator_t;

/**
 * Creates a new agent instruction evaluator instance
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param mut_expr_evaluator The expression evaluator dependency (borrowed reference)
 * @param mut_memory The memory dependency (borrowed reference)
 * @return A newly created evaluator instance or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_agent_instruction_evaluator_t* ar_agent_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory
);

/**
 * Destroys an agent instruction evaluator instance
 * @param own_evaluator The evaluator to destroy (owned)
 * @note Ownership: Takes ownership of the evaluator
 */
void ar_agent_instruction_evaluator__destroy(ar_agent_instruction_evaluator_t *own_evaluator);

/**
 * Evaluates an agent instruction using stored dependencies from instance
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_context The context map (can be NULL)
 * @param ref_ast The instruction AST node
 * @return true if evaluation succeeded, false otherwise
 * @note Ownership: Does not take ownership of any parameters
 */
bool ar_agent_instruction_evaluator__evaluate(
    ar_agent_instruction_evaluator_t *mut_evaluator,
    data_t *ref_context,
    const ar_instruction_ast_t *ref_ast
);



#endif /* AGERUN_AGENT_INSTRUCTION_EVALUATOR_H */
