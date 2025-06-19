/**
 * @file agerun_send_instruction_evaluator.h
 * @brief Send instruction evaluator module for AgeRun
 * 
 * This module provides functionality to evaluate send instruction AST nodes.
 * It handles message sending between agents in the form "send(agent_id, message)".
 */

#ifndef AGERUN_SEND_INSTRUCTION_EVALUATOR_H
#define AGERUN_SEND_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "agerun_instruction_ast.h"
#include "agerun_data.h"
#include "agerun_expression_evaluator.h"

/**
 * Evaluates a send instruction AST node
 * @param mut_expr_evaluator The expression evaluator to use (mutable reference)
 * @param mut_memory The memory map containing variables (mutable reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note The send instruction transfers ownership of the message to the target agent.
 *       If the send has a result assignment, it will modify the memory map.
 */
bool ar__send_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);

#endif /* AGERUN_SEND_INSTRUCTION_EVALUATOR_H */