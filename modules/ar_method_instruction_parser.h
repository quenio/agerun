#ifndef AGERUN_METHOD_INSTRUCTION_PARSER_H
#define AGERUN_METHOD_INSTRUCTION_PARSER_H

#include "ar_instruction_ast.h"
#include "ar_log.h"

/**
 * @file agerun_method_instruction_parser.h
 * @brief Parser for method() function instructions in AgeRun language
 * 
 * This module provides specialized parsing for method() function calls,
 * following the instantiable parser pattern with create/destroy lifecycle.
 */

/* Opaque parser type */
typedef struct ar_method_instruction_parser_s ar_method_instruction_parser_t;

/**
 * Create a new method instruction parser instance
 * @param ref_log Optional log instance for error reporting (borrowed reference, may be NULL)
 * @return A new parser instance, or NULL on allocation failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_method_instruction_parser_t* ar_method_instruction_parser__create(ar_log_t *ref_log);

/**
 * Destroy a method instruction parser instance
 * @param own_parser The parser instance to destroy (takes ownership)
 * @note Ownership: Takes ownership of the parser and destroys it
 */
void ar_method_instruction_parser__destroy(ar_method_instruction_parser_t *own_parser);

/**
 * Parse a method instruction
 * @param mut_parser The parser instance to use (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, may be NULL)
 * @return Parsed AST node, or NULL on error
 * @note Ownership: Returns an owned AST that caller must destroy
 * @note The parser expects exactly 3 arguments: name, code, and version
 */
ar_instruction_ast_t* ar_method_instruction_parser__parse(
    ar_method_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
);

/**
 * Get the last error message from the parser
 * @param ref_parser The parser instance (borrowed reference)
 * @return Error message, or NULL if no error
 * @note Ownership: Returns a borrowed reference valid until next parse or destroy
 */
const char* ar_method_instruction_parser__get_error(const ar_method_instruction_parser_t *ref_parser);

/**
 * Get the position where the last error occurred
 * @param ref_parser The parser instance (borrowed reference)
 * @return Character position of the error, or 0 if no error
 */
size_t ar_method_instruction_parser__get_error_position(const ar_method_instruction_parser_t *ref_parser);

#endif /* AGERUN_METHOD_INSTRUCTION_PARSER_H */