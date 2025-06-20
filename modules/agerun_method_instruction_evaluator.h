/**
 * @file agerun_method_instruction_evaluator.h
 * @brief Public interface for the method instruction evaluator module
 * 
 * This module is responsible for evaluating method instructions (method()).
 * It handles method creation and registration in the methodology system.
 */

#ifndef AGERUN_METHOD_INSTRUCTION_EVALUATOR_H
#define AGERUN_METHOD_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "agerun_expression_evaluator.h"
#include "agerun_data.h"
#include "agerun_instruction_ast.h"

/**
 * Opaque type for method instruction evaluator
 */
typedef struct ar_method_instruction_evaluator_s ar_method_instruction_evaluator_t;

/**
 * Creates a new method instruction evaluator instance
 * 
 * @param ref_expr_evaluator The expression evaluator to use (borrowed reference)
 * @param mut_memory The memory map to access/modify (mutable reference)
 * @return A new method instruction evaluator instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_method_instruction_evaluator_t* ar__method_instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
);

/**
 * Destroys a method instruction evaluator instance
 * 
 * @param own_evaluator The evaluator to destroy (takes ownership)
 */
void ar__method_instruction_evaluator__destroy(
    ar_method_instruction_evaluator_t *own_evaluator
);

/**
 * Evaluates a method instruction using the stored dependencies
 * 
 * The method() instruction takes three string arguments:
 * - method_name: The name of the method to create
 * - instructions: The instruction code for the method
 * - version: The semantic version string (e.g., "1.0.0")
 * 
 * Returns 1 on success, 0 on failure when assigned to a variable.
 * 
 * @param mut_evaluator The evaluator instance to use
 * @param ref_ast The instruction AST node
 * @return true if evaluation succeeded, false otherwise
 */
bool ar__method_instruction_evaluator__evaluate(
    ar_method_instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);

/**
 * Legacy interface for backward compatibility
 * 
 * @param mut_expr_evaluator The expression evaluator to use
 * @param mut_memory The memory map to access/modify
 * @param ref_ast The instruction AST node
 * @return true if evaluation succeeded, false otherwise
 * @note This function will be removed once instruction_evaluator is updated
 */
bool ar_method_instruction_evaluator__evaluate_legacy(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);

#endif /* AGERUN_METHOD_INSTRUCTION_EVALUATOR_H */