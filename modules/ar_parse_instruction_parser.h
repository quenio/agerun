/**
 * @file agerun_parse_instruction_parser.h
 * @brief Parser for parse() function instructions in the AgeRun language
 *
 * This module handles parsing of parse() function calls, which extract values
 * from input strings based on template patterns. It follows the specialized
 * parser pattern with an instantiable parser that has create/destroy lifecycle.
 */

#ifndef AGERUN_PARSE_INSTRUCTION_PARSER_H
#define AGERUN_PARSE_INSTRUCTION_PARSER_H

#include <stddef.h>

/* Dependencies */
#include "ar_instruction_ast.h"

/* Opaque type for parse instruction parser */
typedef struct ar_parse_instruction_parser_s ar_parse_instruction_parser_t;

/**
 * Creates a new parse instruction parser instance
 *
 * @return A new parser instance or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_parse_instruction_parser_t* ar_parse_instruction_parser__create(void);

/**
 * Destroys a parse instruction parser instance
 *
 * @param own_parser The parser to destroy (takes ownership)
 * @note Frees all resources associated with the parser
 */
void ar_parse_instruction_parser__destroy(ar_parse_instruction_parser_t *own_parser);

/**
 * Parses a parse() function instruction
 *
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, may be NULL)
 * @return Parsed AST node or NULL on error
 *
 * @note Ownership: Returns an owned AST that caller must destroy
 * @note The parse() function requires exactly 2 arguments:
 *       1. Template string with {variable} placeholders
 *       2. Input string to parse
 * @note Supports optional assignment: memory.result := parse(...)
 * @note Sets error message and position on parse failure
 */
ar_instruction_ast_t* ar_parse_instruction_parser__parse(
    ar_parse_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
);

/**
 * Gets the last error message from the parser
 *
 * @param ref_parser The parser instance (borrowed reference)
 * @return Error message or NULL if no error
 * @note The returned string is owned by the parser and valid until next parse or destroy
 */
const char* ar_parse_instruction_parser__get_error(const ar_parse_instruction_parser_t *ref_parser);

/**
 * Gets the position where the last error occurred
 *
 * @param ref_parser The parser instance (borrowed reference)
 * @return Character position of the error in the input string
 */
size_t ar_parse_instruction_parser__get_error_position(const ar_parse_instruction_parser_t *ref_parser);

#endif /* AGERUN_PARSE_INSTRUCTION_PARSER_H */