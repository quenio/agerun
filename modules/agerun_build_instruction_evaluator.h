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

/**
 * Evaluates a build instruction
 * 
 * Build instructions construct strings by replacing placeholders with values:
 * - Template: "Hello {name}, you are {age} years old"
 * - Values: {"name": "Alice", "age": 30}
 * - Result: "Hello Alice, you are 30 years old"
 * 
 * @param mut_expr_evaluator Expression evaluator to use (mutable reference)
 * @param mut_memory Memory map to read from and write to (mutable reference)
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
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);

#endif /* AGERUN_BUILD_INSTRUCTION_EVALUATOR_H */