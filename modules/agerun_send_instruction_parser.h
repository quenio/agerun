#ifndef AGERUN_SEND_INSTRUCTION_PARSER_H
#define AGERUN_SEND_INSTRUCTION_PARSER_H

#include "agerun_instruction_ast.h"
#include "agerun_expression_parser.h"

/**
 * Opaque parser structure for send instructions.
 * Handles parsing of send function calls like "send(agent_id, message)".
 */
typedef struct ar_send_instruction_parser_s ar_send_instruction_parser_t;

/**
 * Create a new send instruction parser instance.
 * 
 * @return Newly created parser instance (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_send_instruction_parser_t* ar_send_instruction_parser__create(void);

/**
 * Destroy a send instruction parser instance.
 * 
 * @param own_parser The parser instance to destroy (ownership transferred)
 * @note Ownership: Takes ownership of the parser and destroys it.
 */
void ar_send_instruction_parser__destroy(ar_send_instruction_parser_t *own_parser);

/**
 * Parse a send instruction.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, can be NULL)
 * @return Parsed send AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 * @note The instruction must be of the form "send(agent_id, message)"
 * @note If ref_result_path is provided, it represents assignment like "memory.x := send(...)"
 */
ar_instruction_ast_t* ar_send_instruction_parser__parse(
    ar_send_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
);

/**
 * Get the last error message from the parser.
 * 
 * @param ref_parser The parser instance (borrowed reference)
 * @return The error message (borrowed reference), or NULL if no error
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar_send_instruction_parser__get_error(
    const ar_send_instruction_parser_t *ref_parser
);

/**
 * Get the error position from the last parse attempt.
 * 
 * @param ref_parser The parser instance (borrowed reference)
 * @return The character offset where the error occurred, or 0 if no error
 */
size_t ar_send_instruction_parser__get_error_position(
    const ar_send_instruction_parser_t *ref_parser
);

#endif /* AGERUN_SEND_INSTRUCTION_PARSER_H */