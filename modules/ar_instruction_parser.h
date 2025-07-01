#ifndef AGERUN_INSTRUCTION_PARSER_H
#define AGERUN_INSTRUCTION_PARSER_H

#include <stddef.h>
#include "ar_instruction_ast.h"
#include "ar_log.h"

/**
 * Opaque parser structure.
 * Maintains parsing state and can be reused for multiple instructions.
 */
typedef struct instruction_parser_s instruction_parser_t;

/**
 * Create a new instruction parser instance.
 * 
 * @param ref_log Optional log instance for error reporting (borrowed reference, may be NULL)
 * @return Newly created parser instance (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
instruction_parser_t* ar_instruction_parser__create(ar_log_t *ref_log);

/**
 * Destroy an instruction parser instance.
 * 
 * @param own_parser The parser instance to destroy (ownership transferred)
 * @note Ownership: Takes ownership of the parser and destroys it.
 */
void ar_instruction_parser__destroy(instruction_parser_t *own_parser);

/**
 * Get the last error message from the parser.
 * 
 * @param ref_parser The parser instance (borrowed reference)
 * @return The error message (borrowed reference), or NULL if no error
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar_instruction_parser__get_error(const instruction_parser_t *ref_parser);

/**
 * Get the error position from the last parse attempt.
 * 
 * @param ref_parser The parser instance (borrowed reference)
 * @return The character offset where the error occurred, or 0 if no error
 */
size_t ar_instruction_parser__get_error_position(const instruction_parser_t *ref_parser);

/**
 * Parse an instruction using the unified parser facade.
 * Automatically detects the instruction type and dispatches to the appropriate specialized parser.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @return Parsed instruction AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
ar_instruction_ast_t* ar_instruction_parser__parse(instruction_parser_t *mut_parser, const char *ref_instruction);


#endif /* AGERUN_INSTRUCTION_PARSER_H */
