#ifndef AGERUN_INSTRUCTION_PARSER_H
#define AGERUN_INSTRUCTION_PARSER_H

#include <stddef.h>
#include "agerun_instruction_ast.h"

/**
 * Opaque parser structure.
 * Maintains parsing state and can be reused for multiple instructions.
 */
typedef struct instruction_parser_s instruction_parser_t;

/**
 * Create a new instruction parser instance.
 * 
 * @return Newly created parser instance (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
instruction_parser_t* ar__instruction_parser__create(void);

/**
 * Destroy an instruction parser instance.
 * 
 * @param own_parser The parser instance to destroy (ownership transferred)
 * @note Ownership: Takes ownership of the parser and destroys it.
 */
void ar__instruction_parser__destroy(instruction_parser_t *own_parser);

/**
 * Get the last error message from the parser.
 * 
 * @param ref_parser The parser instance (borrowed reference)
 * @return The error message (borrowed reference), or NULL if no error
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar__instruction_parser__get_error(const instruction_parser_t *ref_parser);

/**
 * Get the error position from the last parse attempt.
 * 
 * @param ref_parser The parser instance (borrowed reference)
 * @return The character offset where the error occurred, or 0 if no error
 */
size_t ar__instruction_parser__get_error_position(const instruction_parser_t *ref_parser);

/**
 * Parse an assignment instruction.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @return Parsed assignment AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
instruction_ast_t* ar__instruction_parser__parse_assignment(instruction_parser_t *mut_parser, const char *ref_instruction);

/**
 * Parse a send function.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, can be NULL)
 * @return Parsed send AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
instruction_ast_t* ar__instruction_parser__parse_send(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path);

/**
 * Parse an if function.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, can be NULL)
 * @return Parsed if AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
instruction_ast_t* ar__instruction_parser__parse_if(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path);

/**
 * Parse a method function.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, can be NULL)
 * @return Parsed method AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
instruction_ast_t* ar__instruction_parser__parse_method(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path);

/**
 * Parse an agent function.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, can be NULL)
 * @return Parsed agent AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
instruction_ast_t* ar__instruction_parser__parse_agent(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path);

/**
 * Parse a destroy function.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, can be NULL)
 * @return Parsed destroy AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
instruction_ast_t* ar__instruction_parser__parse_destroy(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path);

/**
 * Parse a parse function.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, can be NULL)
 * @return Parsed parse AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
instruction_ast_t* ar__instruction_parser__parse_parse(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path);

/**
 * Parse a build function.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @param ref_instruction The instruction string to parse (borrowed reference)
 * @param ref_result_path Optional result assignment path (borrowed reference, can be NULL)
 * @return Parsed build AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
instruction_ast_t* ar__instruction_parser__parse_build(instruction_parser_t *mut_parser, const char *ref_instruction, const char *ref_result_path);

#endif /* AGERUN_INSTRUCTION_PARSER_H */
