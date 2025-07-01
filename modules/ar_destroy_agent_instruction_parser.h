#ifndef AGERUN_DESTROY_AGENT_INSTRUCTION_PARSER_H
#define AGERUN_DESTROY_AGENT_INSTRUCTION_PARSER_H

#include <stddef.h>
#include "ar_instruction_ast.h"
#include "ar_log.h"

/**
 * Opaque destroy agent instruction parser structure.
 */
typedef struct ar_destroy_agent_instruction_parser_s ar_destroy_agent_instruction_parser_t;

/**
 * Creates a new destroy agent instruction parser.
 * 
 * @param ref_log Optional log instance for error reporting (borrowed reference, may be NULL)
 * @return A new parser instance (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_destroy_agent_instruction_parser_t* ar_destroy_agent_instruction_parser__create(ar_log_t *ref_log);

/**
 * Destroys a destroy agent instruction parser.
 * 
 * @param own_parser The parser to destroy (owned reference)
 * @note Ownership: Takes ownership of the parser and destroys it.
 */
void ar_destroy_agent_instruction_parser__destroy(ar_destroy_agent_instruction_parser_t *own_parser);

/**
 * Parses a destroy agent instruction.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, may be NULL)
 * @return AST node representing the instruction (owned by caller), or NULL on error
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The function does not take ownership of any parameters.
 */
ar_instruction_ast_t* ar_destroy_agent_instruction_parser__parse(
    ar_destroy_agent_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
);

/**
 * Gets the last error message from the parser.
 * 
 * @param ref_parser The parser instance (borrowed reference)
 * @return The error message (borrowed reference), or NULL if no error
 * @note Ownership: Returns a borrowed reference that remains valid until
 *       the next parse operation or parser destruction.
 */
const char* ar_destroy_agent_instruction_parser__get_error(
    const ar_destroy_agent_instruction_parser_t *ref_parser
);

/**
 * Gets the position of the last error.
 * 
 * @param ref_parser The parser instance (borrowed reference)
 * @return The error position, or 0 if no error
 */
size_t ar_destroy_agent_instruction_parser__get_error_position(
    const ar_destroy_agent_instruction_parser_t *ref_parser
);

#endif /* AGERUN_DESTROY_AGENT_INSTRUCTION_PARSER_H */