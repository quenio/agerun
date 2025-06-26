#ifndef AR_METHOD_PARSER_H
#define AR_METHOD_PARSER_H

/**
 * @file ar_method_parser.h
 * @brief Method parser - Parses method source code into AST
 * 
 * This module is responsible for parsing method source code and creating
 * the corresponding Abstract Syntax Tree (AST) representation.
 */

#include "ar_method_ast.h"

// Forward declaration of opaque type
typedef struct ar_method_parser_s ar_method_parser_t;

/**
 * @brief Create a new method parser
 * 
 * @return Owned parser instance, or NULL on allocation failure
 * @note Ownership: Caller owns the returned parser
 */
ar_method_parser_t* ar_method_parser__create(void);

/**
 * @brief Destroy a method parser
 * 
 * @param own_parser The parser to destroy (takes ownership)
 * @note Ownership: Takes ownership of the parser and destroys it
 */
void ar_method_parser__destroy(ar_method_parser_t *own_parser);

/**
 * @brief Parse method source code into an AST
 * 
 * @param mut_parser Parser instance to use
 * @param ref_source Source code to parse
 * @return Owned AST on success, NULL on parse error
 * @note Ownership: Caller owns the returned AST
 */
ar_method_ast_t* ar_method_parser__parse(ar_method_parser_t *mut_parser, const char *ref_source);

#endif // AR_METHOD_PARSER_H