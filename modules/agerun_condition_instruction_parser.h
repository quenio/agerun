#ifndef AGERUN_CONDITION_INSTRUCTION_PARSER_H
#define AGERUN_CONDITION_INSTRUCTION_PARSER_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @file agerun_condition_instruction_parser.h
 * @brief Parser for condition (if) instructions in AgeRun
 *
 * This module is responsible for parsing if() function calls in the AgeRun language.
 * It follows the specialized parser pattern, providing an instantiable parser
 * with create/destroy lifecycle.
 *
 * The if() function syntax:
 * - if(condition, then_instruction, else_instruction)
 * - Can be used with assignment: memory.result := if(cond, val1, val2)
 */

/* Forward declaration of opaque parser type */
typedef struct ar_condition_instruction_parser_s ar_condition_instruction_parser_t;

/* Forward declaration of instruction AST type */
typedef struct instruction_ast_s instruction_ast_t;

/**
 * Create a new condition instruction parser.
 * @return A new parser instance, or NULL on allocation failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_condition_instruction_parser_t* ar_condition_instruction_parser__create(void);

/**
 * Destroy a condition instruction parser.
 * @param own_parser The parser to destroy (takes ownership)
 */
void ar_condition_instruction_parser__destroy(ar_condition_instruction_parser_t *own_parser);

/**
 * Parse a condition instruction.
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, can be NULL)
 * @return Parsed instruction AST or NULL on error
 * @note Ownership: Returns an owned value that caller must destroy
 * @note The parser retains error information accessible via get_error functions
 */
instruction_ast_t* ar_condition_instruction_parser__parse(
    ar_condition_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
);

/**
 * Get the last error message from the parser.
 * @param ref_parser The parser to query (borrowed reference)
 * @return Error message or NULL if no error
 * @note Ownership: Returns a borrowed reference, do not free
 */
const char* ar_condition_instruction_parser__get_error(
    const ar_condition_instruction_parser_t *ref_parser
);

/**
 * Get the position where the last error occurred.
 * @param ref_parser The parser to query (borrowed reference)
 * @return Character position of error, or 0 if no error
 */
size_t ar_condition_instruction_parser__get_error_position(
    const ar_condition_instruction_parser_t *ref_parser
);

#endif /* AGERUN_CONDITION_INSTRUCTION_PARSER_H */