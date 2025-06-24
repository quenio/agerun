#ifndef AGERUN_AGENT_INSTRUCTION_PARSER_H
#define AGERUN_AGENT_INSTRUCTION_PARSER_H

#include "ar_instruction_ast.h"

/**
 * @file agerun_agent_instruction_parser.h
 * @brief Parser for agent() function instructions in AgeRun language
 * 
 * This module provides specialized parsing for agent() function calls,
 * following the instantiable parser pattern with create/destroy lifecycle.
 * Supports both 2-parameter (method, version) and 3-parameter (method, version, context) forms.
 */

/* Opaque parser type */
typedef struct ar_agent_instruction_parser_s ar_agent_instruction_parser_t;

/**
 * Create a new agent instruction parser instance
 * @return A new parser instance, or NULL on allocation failure
 * @note Ownership: Returns an owned value that caller must destroy
 */
ar_agent_instruction_parser_t* ar_agent_instruction_parser__create(void);

/**
 * Destroy an agent instruction parser instance
 * @param own_parser The parser instance to destroy (takes ownership)
 * @note Ownership: Takes ownership of the parser and destroys it
 */
void ar_agent_instruction_parser__destroy(ar_agent_instruction_parser_t *own_parser);

/**
 * Parse an agent instruction
 * @param mut_parser The parser instance to use (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, may be NULL)
 * @return Parsed AST node, or NULL on error
 * @note Ownership: Returns an owned AST that caller must destroy
 * @note The parser accepts 2 arguments (method, version) or 3 arguments (method, version, context)
 */
ar_instruction_ast_t* ar_agent_instruction_parser__parse(
    ar_agent_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
);

/**
 * Get the last error message from the parser
 * @param ref_parser The parser instance (borrowed reference)
 * @return Error message, or NULL if no error
 * @note Ownership: Returns a borrowed reference valid until next parse or destroy
 */
const char* ar_agent_instruction_parser__get_error(const ar_agent_instruction_parser_t *ref_parser);

/**
 * Get the position where the last error occurred
 * @param ref_parser The parser instance (borrowed reference)
 * @return Character position of the error, or 0 if no error
 */
size_t ar_agent_instruction_parser__get_error_position(const ar_agent_instruction_parser_t *ref_parser);

#endif /* AGERUN_AGENT_INSTRUCTION_PARSER_H */