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

/**
 * Evaluates an agent instruction
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
 */
bool ar__agent_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    data_t *ref_context,
    const instruction_ast_t *ref_ast
);

#endif /* AGERUN_AGENT_INSTRUCTION_EVALUATOR_H */