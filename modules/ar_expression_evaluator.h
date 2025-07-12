/**
 * @file ar_expression_evaluator.h
 * @brief Expression evaluator module for AgeRun
 * 
 * This module provides functionality to evaluate expression AST nodes
 * into ar_data_t values. It handles all expression types including
 * literals, memory access, and binary operations.
 */

#ifndef AGERUN_EXPRESSION_EVALUATOR_H
#define AGERUN_EXPRESSION_EVALUATOR_H

#include "ar_expression_ast.h"
#include "ar_data.h"
#include "ar_log.h"
#include "ar_frame.h"

/**
 * Opaque type for expression evaluator
 */
typedef struct ar_expression_evaluator_s ar_expression_evaluator_t;

/**
 * Creates a new expression evaluator
 * @param ref_log The log instance for error reporting (borrowed reference)
 * @return A new evaluator instance
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_expression_evaluator_t* ar_expression_evaluator__create(
    ar_log_t *ref_log
);

/**
 * Destroys an expression evaluator
 * @param own_evaluator The evaluator to destroy
 * @note Ownership: Takes ownership and destroys the evaluator
 */
void ar_expression_evaluator__destroy(ar_expression_evaluator_t *own_evaluator);

/**
 * Evaluates any expression AST node using frame context
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_frame The frame containing memory/context/message (borrowed reference)
 * @param ref_ast The expression AST to evaluate (borrowed reference)
 * @return The evaluated result
 * @note Ownership: 
 *       - Memory access: Returns borrowed reference (owned by frame's memory/context map)
 *       - Literals: Returns owned value that caller must destroy
 *       - Operations: Returns owned value that caller must destroy
 */
ar_data_t* ar_expression_evaluator__evaluate(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_frame_t *ref_frame,
    const ar_expression_ast_t *ref_ast
);

#endif /* AGERUN_EXPRESSION_EVALUATOR_H */
