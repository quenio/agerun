#ifndef AGERUN_TAIL_INSTRUCTION_PARSER_H
#define AGERUN_TAIL_INSTRUCTION_PARSER_H

#include <stddef.h>
#include "ar_instruction_ast.h"
#include "ar_log.h"

/**
 * Opaque parser structure for tail instructions.
 */
typedef struct ar_tail_instruction_parser_s ar_tail_instruction_parser_t;

/**
 * Create a new tail instruction parser instance.
 * @param ref_log The log instance for error reporting (borrowed reference)
 * @return Newly created parser instance (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_tail_instruction_parser_t* ar_tail_instruction_parser__create(ar_log_t *ref_log);

/**
 * Destroy a tail instruction parser instance.
 * @param own_parser The parser instance to destroy (ownership transferred)
 * @note Ownership: Takes ownership of the parser and destroys it.
 */
void ar_tail_instruction_parser__destroy(ar_tail_instruction_parser_t *own_parser);

/**
 * Parse a tail instruction.
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, can be NULL)
 * @return Parsed tail AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
ar_instruction_ast_t* ar_tail_instruction_parser__parse(
    ar_tail_instruction_parser_t *mut_parser,
    const char *ref_instruction,
    const char *ref_result_path
);

/**
 * Get the last error message from the parser.
 * DEPRECATED: Use ar_log for error reporting.
 * @param ref_parser The parser instance (borrowed reference)
 * @return Always returns NULL
 */
const char* ar_tail_instruction_parser__get_error(
    const ar_tail_instruction_parser_t *ref_parser
);

/**
 * Get the error position from the last parse attempt.
 * DEPRECATED: Use ar_log for error reporting.
 * @param ref_parser The parser instance (borrowed reference)
 * @return Always returns 0
 */
size_t ar_tail_instruction_parser__get_error_position(
    const ar_tail_instruction_parser_t *ref_parser
);

#endif /* AGERUN_TAIL_INSTRUCTION_PARSER_H */
