#ifndef AGERUN_ASSIGNMENT_INSTRUCTION_PARSER_H
#define AGERUN_ASSIGNMENT_INSTRUCTION_PARSER_H

#include "ar_instruction_ast.h"
#include "ar_expression_parser.h"
#include "ar_log.h"

/**
 * Opaque parser structure for assignment instructions.
 * Handles parsing of memory assignment instructions like "memory.x := 42".
 */
typedef struct ar_assignment_instruction_parser_s ar_assignment_instruction_parser_t;

/**
 * Create a new assignment instruction parser instance.
 * 
 * @param ref_log The log instance for error reporting (borrowed reference)
 * @return Newly created parser instance (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_assignment_instruction_parser_t* ar_assignment_instruction_parser__create(ar_log_t *ref_log);

/**
 * Destroy an assignment instruction parser instance.
 * 
 * @param own_parser The parser instance to destroy (ownership transferred)
 * @note Ownership: Takes ownership of the parser and destroys it.
 */
void ar_assignment_instruction_parser__destroy(ar_assignment_instruction_parser_t *own_parser);

/**
 * Parse an assignment instruction.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @return Parsed assignment AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 * @note The instruction must be of the form "memory.path := expression"
 */
ar_instruction_ast_t* ar_assignment_instruction_parser__parse(
    ar_assignment_instruction_parser_t *mut_parser,
    const char *ref_instruction
);


/**
 * Get the last error message from the parser.
 * DEPRECATED: This function always returns NULL. Use ar_log for error reporting.
 * 
 * @param ref_parser The parser instance (borrowed reference)
 * @return Always returns NULL
 */
const char* ar_assignment_instruction_parser__get_error(
    const ar_assignment_instruction_parser_t *ref_parser
);

/**
 * Get the error position from the last parse attempt.
 * DEPRECATED: This function always returns 0. Use ar_log for error reporting.
 * 
 * @param ref_parser The parser instance (borrowed reference)
 * @return Always returns 0
 */
size_t ar_assignment_instruction_parser__get_error_position(
    const ar_assignment_instruction_parser_t *ref_parser
);

#endif /* AGERUN_ASSIGNMENT_INSTRUCTION_PARSER_H */