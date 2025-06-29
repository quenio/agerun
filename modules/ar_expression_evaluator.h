/**
 * @file agerun_expression_evaluator.h
 * @brief Expression evaluator module for AgeRun
 * 
 * This module provides functionality to evaluate expression AST nodes
 * into data_t values. It handles all expression types including
 * literals, memory access, and binary operations.
 */

#ifndef AGERUN_EXPRESSION_EVALUATOR_H
#define AGERUN_EXPRESSION_EVALUATOR_H

#include "ar_expression_ast.h"
#include "ar_data.h"

/**
 * Opaque type for expression evaluator
 */
typedef struct expression_evaluator_s ar_expression_evaluator_t;

/**
 * Creates a new expression evaluator
 * @param ref_memory The memory map containing variables (borrowed reference)
 * @param ref_context Optional context map with additional data (borrowed reference, can be NULL)
 * @return A new evaluator instance
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_expression_evaluator_t* ar__expression_evaluator__create(
    data_t *ref_memory,
    data_t *ref_context
);

/**
 * Destroys an expression evaluator
 * @param own_evaluator The evaluator to destroy
 * @note Ownership: Takes ownership and destroys the evaluator
 */
void ar__expression_evaluator__destroy(ar_expression_evaluator_t *own_evaluator);

/**
 * Evaluates an integer literal AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_node The AST node to evaluate (borrowed reference)
 * @return The integer value as data_t
 * @note Ownership: Returns an owned value that caller must destroy
 *       Returns NULL if node is not an integer literal
 */
data_t* ar__expression_evaluator__evaluate_literal_int(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node
);

/**
 * Evaluates a double literal AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_node The AST node to evaluate (borrowed reference)
 * @return The double value as data_t
 * @note Ownership: Returns an owned value that caller must destroy
 *       Returns NULL if node is not a double literal
 */
data_t* ar__expression_evaluator__evaluate_literal_double(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node
);

/**
 * Evaluates a string literal AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_node The AST node to evaluate (borrowed reference)
 * @return The string value as data_t
 * @note Ownership: Returns an owned value that caller must destroy
 *       Returns NULL if node is not a string literal
 */
data_t* ar__expression_evaluator__evaluate_literal_string(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node
);

/**
 * Evaluates a memory access AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_node The AST node to evaluate (borrowed reference)
 * @return The accessed value as data_t
 * @note Ownership: Returns an owned value that caller must destroy
 *       Returns NULL if variable not found or node is not a memory access
 */
data_t* ar__expression_evaluator__evaluate_memory_access(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node
);

/**
 * Evaluates a binary operation AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_node The AST node to evaluate (borrowed reference)
 * @return The operation result as data_t
 * @note Ownership: Returns an owned value that caller must destroy
 *       Returns NULL on evaluation errors or if node is not a binary operation
 */
data_t* ar__expression_evaluator__evaluate_binary_op(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_node
);

/**
 * Evaluates any expression AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The expression AST to evaluate (borrowed reference)
 * @return The evaluated result
 * @note Ownership: 
 *       - Memory access: Returns borrowed reference (owned by memory/context map)
 *       - Literals: Returns owned value that caller must destroy
 *       - Operations: Returns owned value that caller must destroy
 */
data_t* ar__expression_evaluator__evaluate(
    ar_expression_evaluator_t *mut_evaluator,
    const ar_expression_ast_t *ref_ast
);

#endif /* AGERUN_EXPRESSION_EVALUATOR_H */
