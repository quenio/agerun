#ifndef AGERUN_EXPRESSION_PARSER_H
#define AGERUN_EXPRESSION_PARSER_H

#include "agerun_expression_ast.h"
#include <stdbool.h>

/**
 * Opaque parser structure.
 * Maintains parsing state including the expression string and current position.
 */
typedef struct expression_parser_s expression_parser_t;

/**
 * Create a new expression parser instance.
 * 
 * @param ref_expression The expression string to parse (borrowed reference)
 * @return Newly created parser instance (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The parser makes a copy of the expression string.
 */
expression_parser_t* ar__expression_parser__create(const char *ref_expression);

/**
 * Destroy an expression parser instance.
 * 
 * @param own_parser The parser instance to destroy (ownership transferred)
 * @note Ownership: Takes ownership of the parser and destroys it.
 */
void ar__expression_parser__destroy(expression_parser_t *own_parser);

/**
 * Get the current position in the expression being parsed.
 * 
 * @param ref_parser The parser instance (borrowed reference)
 * @return The current character offset in the expression
 */
size_t ar__expression_parser__get_position(const expression_parser_t *ref_parser);

/**
 * Get the last error message from the parser.
 * 
 * @param ref_parser The parser instance (borrowed reference)
 * @return The error message (borrowed reference), or NULL if no error
 * @note Ownership: Returns a borrowed reference. Do not free.
 */
const char* ar__expression_parser__get_error(const expression_parser_t *ref_parser);

/**
 * Parse an expression and return the AST.
 * This is the main entry point for parsing.
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @return Parsed expression AST (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST that caller must destroy.
 *       On failure, use ar__expression_parser__get_error() for details.
 *       This function ensures the entire input is consumed.
 */
expression_ast_t* ar__expression_parser__parse_expression(expression_parser_t *mut_parser);

/**
 * Parse a literal (integer, double, or string).
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @return Parsed literal AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
expression_ast_t* ar__expression_parser__parse_literal(expression_parser_t *mut_parser);

/**
 * Parse a memory access expression (e.g., memory.x, message.content).
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @return Parsed memory access AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
expression_ast_t* ar__expression_parser__parse_memory_access(expression_parser_t *mut_parser);

/**
 * Parse an arithmetic expression (e.g., 2 + 3, x * 5).
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @return Parsed arithmetic AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
expression_ast_t* ar__expression_parser__parse_arithmetic(expression_parser_t *mut_parser);

/**
 * Parse a comparison expression (e.g., x > 5, name = "test").
 * 
 * @param mut_parser The parser instance (mutable reference)
 * @return Parsed comparison AST node (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned AST node that caller must destroy.
 */
expression_ast_t* ar__expression_parser__parse_comparison(expression_parser_t *mut_parser);

#endif /* AGERUN_EXPRESSION_PARSER_H */
