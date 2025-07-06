/**
 * @file agerun_assignment_instruction_evaluator.h
 * @brief Assignment instruction evaluator module for AgeRun
 * 
 * This module provides functionality to evaluate assignment instruction AST nodes.
 * It handles variable assignments in the form "memory.variable := expression".
 */

#ifndef AGERUN_ASSIGNMENT_INSTRUCTION_EVALUATOR_H
#define AGERUN_ASSIGNMENT_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "ar_instruction_ast.h"
#include "ar_data.h"
#include "ar_expression_evaluator.h"
#include "ar_log.h"

/**
 * Opaque type for the assignment instruction evaluator
 */
typedef struct ar_assignment_instruction_evaluator_s ar_assignment_instruction_evaluator_t;

/**
 * Creates a new assignment instruction evaluator
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param ref_expr_evaluator The expression evaluator to use (borrowed reference)
 * @param mut_memory The memory map to modify (mutable reference)
 * @return A newly created evaluator or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_assignment_instruction_evaluator_t* ar_assignment_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    ar_data_t *mut_memory
);

/**
 * Destroys an assignment instruction evaluator
 * @param own_evaluator The evaluator to destroy (owned)
 * @note Takes ownership of the evaluator and frees all resources
 */
void ar_assignment_instruction_evaluator__destroy(
    ar_assignment_instruction_evaluator_t *own_evaluator
);

/**
 * Evaluates an assignment instruction AST node
 * @param mut_evaluator The assignment instruction evaluator to use (mutable reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note The assignment will modify the memory map by storing the evaluated expression result
 */
bool ar_assignment_instruction_evaluator__evaluate(
    ar_assignment_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
);

#endif /* AGERUN_ASSIGNMENT_INSTRUCTION_EVALUATOR_H */
