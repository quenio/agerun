#ifndef AGERUN_CONDITION_INSTRUCTION_PARSER_H
#define AGERUN_CONDITION_INSTRUCTION_PARSER_H

#include <stddef.h>
#include <stdbool.h>
#include "ar_log.h"

/**
 * @file ar_condition_instruction_parser.h
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
typedef struct ar_instruction_ast_s ar_instruction_ast_t;

/**
 * Create a new condition instruction parser.
 * @param ref_log Optional log instance for error reporting (borrowed reference, may be NULL)
 * @return A new parser instance, or NULL on allocation failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_condition_instruction_parser_t* ar_condition_instruction_parser__create(ar_log_t *ref_log);

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
ar_instruction_ast_t* ar_condition_instruction_parser__parse(
    ar_condition_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
);

/**
 * Gets the last error message from the parser.
 * DEPRECATED: Use ar_log for error reporting.
 * @param ref_parser The parser instance (borrowed reference)
 * @return The last error message or NULL if no error occurred
 */
const char* ar_condition_instruction_parser__get_error(const ar_condition_instruction_parser_t *ref_parser);

/**
 * Gets the position where the last error occurred.
 * DEPRECATED: Use ar_log for error reporting.
 * @param ref_parser The parser instance (borrowed reference)
 * @return The character position where the error occurred, or 0 if no error
 */
size_t ar_condition_instruction_parser__get_error_position(const ar_condition_instruction_parser_t *ref_parser);

#endif /* AGERUN_CONDITION_INSTRUCTION_PARSER_H */