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

/**
 * Evaluates a parse instruction
 * 
 * Parse instructions extract values from input strings based on template patterns:
 * - Template: "Hello {name}, you are {age} years old"
 * - Input: "Hello Alice, you are 30 years old"
 * - Result: {"name": "Alice", "age": 30}
 * 
 * @param mut_expr_evaluator Expression evaluator to use (mutable reference)
 * @param mut_memory Memory map to read from and write to (mutable reference)
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
    expression_evaluator_t *mut_expr_evaluator,
    data_t *mut_memory,
    const instruction_ast_t *ref_ast
);

#endif /* AGERUN_PARSE_INSTRUCTION_EVALUATOR_H */