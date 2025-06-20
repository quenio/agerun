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
#include "agerun_expression_evaluator.h"
#include "agerun_data.h"
#include "agerun_instruction_ast.h"

/* Forward declaration of opaque struct */
typedef struct ar_agent_instruction_evaluator_s ar_agent_instruction_evaluator_t;

/**
 * Creates a new agent instruction evaluator instance
 * @param mut_expr_evaluator The expression evaluator dependency (borrowed reference)
 * @param mut_memory The memory dependency (borrowed reference)
 * @return A newly created evaluator instance or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_agent_instruction_evaluator_t* ar__agent_instruction_evaluator__create(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory
);

/**
 * Destroys an agent instruction evaluator instance
 * @param own_evaluator The evaluator to destroy (owned)
 * @note Ownership: Takes ownership of the evaluator
 */
void ar__agent_instruction_evaluator__destroy(ar_agent_instruction_evaluator_t *own_evaluator);

/**
 * Evaluates an agent instruction using stored dependencies from instance
 * @param ref_evaluator The evaluator instance (borrowed reference)
 * @param ref_context The context map (can be NULL)
 * @param ref_ast The instruction AST node
 * @return true if evaluation succeeded, false otherwise
 * @note Ownership: Does not take ownership of any parameters
 */
bool ar__agent_instruction_evaluator__evaluate(
    const ar_agent_instruction_evaluator_t *ref_evaluator,
    data_t *ref_context,
    const instruction_ast_t *ref_ast
);

/**
 * Evaluates an agent instruction (legacy function for backward compatibility)
 * 
 * The agent() instruction takes three arguments:
 * - method_name: The name of the method to use (string)
 * - version: The version of the method (string)
 * - context: The initial context for the agent (map)
 * 
 * Returns the agent ID (integer > 0) on success, 0 on failure when assigned to a variable.
 * 
 * @param mut_expr_evaluator The expression evaluator to use
 * @param mut_memory The memory map to access/modify
 * @param ref_context The context map (can be NULL)
 * @param ref_ast The instruction AST node
 * @return true if evaluation succeeded, false otherwise
 * @note This function is deprecated in favor of the instance-based approach
 */
bool ar__agent_instruction_evaluator__evaluate_legacy(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    data_t *ref_context,
    const instruction_ast_t *ref_ast
);

#endif /* AGERUN_AGENT_INSTRUCTION_EVALUATOR_H */