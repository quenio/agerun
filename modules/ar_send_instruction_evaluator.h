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
#include "ar_instruction_ast.h"
#include "ar_data.h"
#include "ar_expression_evaluator.h"

/**
 * Opaque type for send instruction evaluator
 */
typedef struct ar_send_instruction_evaluator_s ar_send_instruction_evaluator_t;

/**
 * Creates a new send instruction evaluator
 * @param ref_expr_evaluator The expression evaluator to use (borrowed reference)
 * @param mut_memory The memory map to use (mutable reference)
 * @return A new send instruction evaluator, or NULL on error
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_send_instruction_evaluator_t* ar_send_instruction_evaluator__create(
    ar_expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
);

/**
 * Destroys a send instruction evaluator
 * @param own_evaluator The evaluator to destroy (takes ownership)
 */
void ar_send_instruction_evaluator__destroy(
    ar_send_instruction_evaluator_t *own_evaluator
);

/**
 * Evaluates a send instruction AST node using stored dependencies
 * @param mut_evaluator The send instruction evaluator to use (mutable reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note The send instruction transfers ownership of the message to the target agent.
 *       If the send has a result assignment, it will modify the memory map.
 */
bool ar_send_instruction_evaluator__evaluate(
    ar_send_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
);

/**
 * Get the last error message from the evaluator
 * @param ref_evaluator The evaluator to get the error from
 * @return Error message string, or NULL if no error
 * @note Ownership: Returns a borrowed reference, do not free
 */
const char* ar_send_instruction_evaluator__get_error(
    const ar_send_instruction_evaluator_t *ref_evaluator
);

#endif /* AGERUN_SEND_INSTRUCTION_EVALUATOR_H */
