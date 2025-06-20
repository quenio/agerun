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
typedef struct expression_evaluator_s expression_evaluator_t;
typedef struct data_s data_t;
typedef struct instruction_ast_s instruction_ast_t;

/* Opaque type for parse instruction evaluator */
typedef struct ar_parse_instruction_evaluator_s parse_instruction_evaluator_t;

/**
 * Creates a new parse instruction evaluator
 * 
 * @param ref_expr_evaluator Expression evaluator to use (borrowed reference)
 * @param mut_memory Memory map to use (mutable reference)
 * @return A new parse instruction evaluator or NULL on failure
 * 
 * @note Ownership: Returns an owned value that caller must destroy
 * @note The evaluator stores references to the provided dependencies
 */
parse_instruction_evaluator_t* ar__parse_instruction_evaluator__create(
    expression_evaluator_t *ref_expr_evaluator,
    data_t *mut_memory
);

/**
 * Destroys a parse instruction evaluator
 * 
 * @param own_evaluator The evaluator to destroy (takes ownership)
 * 
 * @note Frees all resources associated with the evaluator
 * @note Does not destroy the stored dependency references
 */
void ar__parse_instruction_evaluator__destroy(
    parse_instruction_evaluator_t *own_evaluator
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
bool ar__parse_instruction_evaluator__evaluate(
    parse_instruction_evaluator_t *mut_evaluator,
    const instruction_ast_t *ref_ast
);

/**
 * Evaluates a parse instruction (legacy interface)
 * 
 * This function provides backward compatibility until instruction_evaluator
 * is updated to use the instantiable pattern.
 * 
 * @param mut_expr_evaluator Expression evaluator to use (mutable reference)
 * @param mut_memory Memory map to read from and write to (mutable reference)
 * @param ref_ast The parse instruction AST node (borrowed reference)
 * @return true if parse was successful, false otherwise
 * 
 * @note This is a wrapper that creates a temporary evaluator instance
 * @note Will be removed once instruction_evaluator is updated
 */
bool ar_parse_instruction_evaluator__evaluate_legacy(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);

/**
 * Evaluates a parse instruction (old name for compatibility)
 * 
 * @deprecated Use ar_parse_instruction_evaluator__evaluate_legacy instead
 */
bool ar_parse_instruction_evaluator__evaluate(
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);

#endif /* AGERUN_PARSE_INSTRUCTION_EVALUATOR_H */