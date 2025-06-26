#ifndef AGERUN_METHOD_AST_H
#define AGERUN_METHOD_AST_H

#include <stdbool.h>
#include <stddef.h>
#include "ar_instruction_ast.h"

/**
 * Opaque method AST structure.
 * Represents a parsed method as a collection of instruction ASTs.
 */
typedef struct ar_method_ast_s ar_method_ast_t;

/**
 * Create a new method AST.
 * 
 * @return Newly created method AST (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_method_ast_t* ar__method_ast__create(void);

/**
 * Destroy a method AST and all its instruction ASTs.
 * 
 * @param own_ast The method AST to destroy (ownership transferred)
 * @note Ownership: Takes ownership of the AST and destroys it.
 *       Also destroys all instruction ASTs contained within.
 *       Safe to call with NULL.
 */
void ar__method_ast__destroy(ar_method_ast_t* own_ast);

#endif /* AGERUN_METHOD_AST_H */