/**
 * @file agerun_destroy_method_instruction_evaluator.h
 * @brief Public interface for the destroy method instruction evaluator module
 */

#ifndef AGERUN_DESTROY_METHOD_INSTRUCTION_EVALUATOR_H
#define AGERUN_DESTROY_METHOD_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>
#include "agerun_expression_evaluator.h"
#include "agerun_instruction_ast.h"
#include "agerun_data.h"

/* Forward declaration of opaque struct */
typedef struct ar_destroy_method_instruction_evaluator_s ar_destroy_method_instruction_evaluator_t;

/**
 * Creates a new destroy method instruction evaluator instance
 * @param mut_expr_evaluator Expression evaluator to use (mutable reference)
 * @param mut_memory Memory data structure (mutable reference)
 * @return New evaluator instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_destroy_method_instruction_evaluator_t* ar_destroy_method_instruction_evaluator__create(
    ar_expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory
);

/**
 * Destroys a destroy method instruction evaluator instance
 * @param own_evaluator The evaluator to destroy (owned value)
 * @note Ownership: Takes ownership and destroys the evaluator
 */
void ar_destroy_method_instruction_evaluator__destroy(ar_destroy_method_instruction_evaluator_t *own_evaluator);

/**
 * Evaluates a destroy method instruction using stored dependencies
 * @param ref_evaluator The evaluator instance (borrowed reference)
 * @param ref_ast The instruction AST to evaluate (borrowed reference)
 * @return true if evaluation succeeded, false otherwise
 * @note Ownership: Borrows all parameters, does not take ownership
 */
bool ar_destroy_method_instruction_evaluator__evaluate(
    const ar_destroy_method_instruction_evaluator_t *ref_evaluator,
    const ar_instruction_ast_t *ref_ast
);


#endif /* AGERUN_DESTROY_METHOD_INSTRUCTION_EVALUATOR_H */
