/**
 * @file ar_send_instruction_evaluator.h
 * @brief Send instruction evaluator module for AgeRun
 *
 * This module provides functionality to evaluate send instruction AST nodes.
 * It handles message sending between agents in the form "send(agent_id, message)".
 */

#ifndef AGERUN_SEND_INSTRUCTION_EVALUATOR_H
#define AGERUN_SEND_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_instruction_ast.h"
#include "ar_data.h"
#include "ar_expression_evaluator.h"
#include "ar_log.h"
#include "ar_frame.h"

/**
 * Opaque type for send instruction evaluator
 */
typedef struct ar_send_instruction_evaluator_s ar_send_instruction_evaluator_t;

/**
 * Creates a new send instruction evaluator
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param ref_expr_evaluator The expression evaluator to use (borrowed reference)
 * @return A new send instruction evaluator, or NULL on error
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_send_instruction_evaluator_t* ar_send_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator
);

/**
 * Destroys a send instruction evaluator
 * @param own_evaluator The evaluator to destroy (takes ownership)
 */
void ar_send_instruction_evaluator__destroy(
    ar_send_instruction_evaluator_t *own_evaluator
);

/**
 * Evaluates a send instruction AST node using frame-based execution
 * @param ref_evaluator The send instruction evaluator to use (borrowed reference)
 * @param ref_frame The execution frame containing memory and context (borrowed reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note The send instruction transfers ownership of the message to the target agent.
 *       If the send has a result assignment, it will modify the memory map.
 */
bool ar_send_instruction_evaluator__evaluate(
    const ar_send_instruction_evaluator_t *ref_evaluator,
    const ar_frame_t *ref_frame,
    const ar_instruction_ast_t *ref_ast
);


#endif /* AGERUN_SEND_INSTRUCTION_EVALUATOR_H */
