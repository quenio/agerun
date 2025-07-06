/**
 * @file agerun_parse_instruction_evaluator.h
 * @brief Parse instruction evaluator for the instruction evaluator module
 *
 * This module handles the evaluation of parse instructions, which extract values
 * from input strings based on template patterns.
 */

#ifndef AGERUN_PARSE_INSTRUCTION_EVALUATOR_H
#define AGERUN_PARSE_INSTRUCTION_EVALUATOR_H

#include <stdbool.h>

/* Forward declarations */
typedef struct ar_expression_evaluator_s ar_expression_evaluator_t;
typedef struct ar_data_s ar_data_t;
typedef struct ar_instruction_ast_s ar_instruction_ast_t;
typedef struct ar_log_s ar_log_t;

/* Opaque type for parse instruction evaluator */
typedef struct ar_parse_instruction_evaluator_s ar_parse_instruction_evaluator_t;

/**
 * Creates a new parse instruction evaluator
 *
 * @param ref_log The log instance to use for error reporting (borrowed reference)
 * @param ref_expr_evaluator Expression evaluator to use (borrowed reference)
 * @param mut_memory Memory map to use (mutable reference)
 * @return A new parse instruction evaluator or NULL on failure
 *
 * @note Ownership: Returns an owned value that caller must destroy
 * @note The evaluator stores references to the provided dependencies
 */
ar_parse_instruction_evaluator_t* ar_parse_instruction_evaluator__create(
    ar_log_t *ref_log,
    ar_expression_evaluator_t *ref_expr_evaluator,
    ar_data_t *mut_memory
);

/**
 * Destroys a parse instruction evaluator
 *
 * @param own_evaluator The evaluator to destroy (takes ownership)
 *
 * @note Frees all resources associated with the evaluator
 * @note Does not destroy the stored dependency references
 */
void ar_parse_instruction_evaluator__destroy(
    ar_parse_instruction_evaluator_t *own_evaluator
);

/**
 * Evaluates a parse instruction using the stored dependencies
 *
 * Parse instructions extract values from input strings based on template patterns:
 * - Template: "Hello {name}, you are {age} years old"
 * - Input: "Hello Alice, you are 30 years old"
 * - Result: {"name": "Alice", "age": 30}
 *
 * @param mut_evaluator The evaluator instance (mutable reference)
 * @param ref_ast The parse instruction AST node (borrowed reference)
 * @return true if parse was successful, false otherwise
 *
 * @note The parse instruction takes 2 arguments:
 *       1. Template string with {variable} placeholders
 *       2. Input string to parse
 * @note Automatically detects types (integer, double, string) for extracted values
 * @note Returns empty map if template doesn't match input
 * @note Supports optional result assignment: result = parse(template, input)
 */
bool ar_parse_instruction_evaluator__evaluate(
    ar_parse_instruction_evaluator_t *mut_evaluator,
    const ar_instruction_ast_t *ref_ast
);



#endif /* AGERUN_PARSE_INSTRUCTION_EVALUATOR_H */
