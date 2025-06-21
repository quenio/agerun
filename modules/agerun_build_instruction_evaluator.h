/**
 * @file agerun_build_instruction_evaluator.h
 * @brief Build instruction evaluator for the instruction evaluator module
 *
 * This module handles the evaluation of build instructions, which construct
 * strings by replacing placeholders in templates with values from a map.
 */

#ifndef AGERUN_BUILD_INSTRUCTION_EVALUATOR_H
#define AGERUN_BUILD_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>

/* Forward declarations */
typedef struct expression_evaluator_s expression_evaluator_t;
typedef struct data_s data_t;
typedef struct instruction_ast_s instruction_ast_t;

/* Opaque type for build instruction evaluator */
typedef struct ar_build_instruction_evaluator_s ar_build_instruction_evaluator_t;

/**
 * Creates a new build instruction evaluator
 *
 * @param ref_expr_evaluator Expression evaluator to use (borrowed reference)
 * @param mut_memory Memory map to use (mutable reference)
 * @return A new build instruction evaluator or NULL on failure
 *
 * @note Ownership: Returns an owned value that caller must destroy
 * @note The evaluator stores references to the provided dependencies
 */
ar_build_instruction_evaluator_t* ar_build_instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
);

/**
 * Destroys a build instruction evaluator
 *
 * @param own_evaluator The evaluator to destroy (takes ownership)
 *
 * @note Frees all resources associated with the evaluator
 * @note Does not destroy the stored dependency references
 */
void ar_build_instruction_evaluator__destroy(
    ar_build_instruction_evaluator_t *own_evaluator
);

/**
 * Evaluates a build instruction using the stored dependencies
 *
 * Build instructions construct strings by replacing placeholders with values:
 * - Template: "Hello {name}, you are {age} years old"
 * - Values: {"name": "Alice", "age": 30}
 * - Result: "Hello Alice, you are 30 years old"
 *
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The build instruction AST node (borrowed reference)
 * @return true if build was successful, false otherwise
 *
 * @note The build instruction takes 2 arguments:
 *       1. Template string with {variable} placeholders
 *       2. Map of values to substitute
 * @note Values are converted to strings (integers, doubles, strings)
 * @note Placeholders without matching values are kept as-is
 * @note Supports optional result assignment: result = build(template, values)
 */
bool ar_build_instruction_evaluator__evaluate(
    ar_build_instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);

/**
 * Evaluates a build instruction (legacy interface)
 * @param mut_expr_evaluator The expression evaluator
 * @param mut_memory The memory map
 * @param ref_ast The instruction AST node
 * @return true if evaluation succeeded, false otherwise
 * @note Ownership: Does not take ownership of any parameters
 */
bool ar_build_instruction_evaluator__evaluate_legacy(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);

#endif /* AGERUN_BUILD_INSTRUCTION_EVALUATOR_H */
