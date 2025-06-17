/**
 * @file agerun_instruction_evaluator.h
 * @brief Instruction evaluator module for AgeRun
 * 
 * This module provides functionality to evaluate instruction AST nodes
 * and execute them within the AgeRun runtime. It handles all instruction
 * types including assignments, function calls, and control flow.
 * 
 * The evaluator works with instruction AST nodes (from instruction_ast module)
 * and uses the expression_evaluator for evaluating expressions within
 * instructions.
 */

#ifndef AGERUN_INSTRUCTION_EVALUATOR_H
#define AGERUN_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "agerun_instruction_ast.h"
#include "agerun_data.h"
#include "agerun_expression_evaluator.h"

/**
 * Opaque type for instruction evaluator
 */
typedef struct instruction_evaluator_s instruction_evaluator_t;

/**
 * Creates a new instruction evaluator
 * @param ref_expr_evaluator The expression evaluator to use for evaluating expressions (borrowed reference)
 * @param mut_memory The memory map containing variables (mutable reference)
 * @param ref_context Optional context map with additional data (borrowed reference, can be NULL)
 * @param ref_message Optional message being processed (borrowed reference, can be NULL)
 * @return A new evaluator instance
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The function does not take ownership of any parameters.
 */
instruction_evaluator_t* ar__instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory,
    data_t *ref_context,
    data_t *ref_message
);

/**
 * Destroys an instruction evaluator
 * @param own_evaluator The evaluator to destroy
 * @note Ownership: Takes ownership and destroys the evaluator
 */
void ar__instruction_evaluator__destroy(instruction_evaluator_t *own_evaluator);

/**
 * Evaluates an assignment instruction AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note The assignment will modify the memory stored in the evaluator
 */
bool ar__instruction_evaluator__evaluate_assignment(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);

/**
 * Evaluates a send function call AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note If the send has a result assignment, it will modify the memory stored in the evaluator
 */
bool ar__instruction_evaluator__evaluate_send(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);

/**
 * Evaluates an if function call AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note If the if has a result assignment, it will modify the memory stored in the evaluator
 */
bool ar__instruction_evaluator__evaluate_if(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);

/**
 * Evaluates a parse function call AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note If the parse has a result assignment, it will modify the memory stored in the evaluator
 */
bool ar__instruction_evaluator__evaluate_parse(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);

/**
 * Evaluates a build function call AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note If the build has a result assignment, it will modify the memory stored in the evaluator
 */
bool ar__instruction_evaluator__evaluate_build(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);

/**
 * Evaluates a method function call AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note If the method has a result assignment, it will modify the memory stored in the evaluator
 */
bool ar__instruction_evaluator__evaluate_method(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);

/**
 * Evaluates an agent function call AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note If the agent has a result assignment, it will modify the memory stored in the evaluator
 */
bool ar__instruction_evaluator__evaluate_agent(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);

/**
 * Evaluates a destroy function call AST node
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The AST node to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note If the destroy has a result assignment, it will modify the memory stored in the evaluator
 */
bool ar__instruction_evaluator__evaluate_destroy(
    instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);

#endif /* AGERUN_INSTRUCTION_EVALUATOR_H */