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

/**
 * Add an instruction AST to the method AST.
 * 
 * @param mut_ast The method AST to add to (mutable reference)
 * @param own_instruction The instruction AST to add (ownership transferred)
 * @note Ownership: Takes ownership of the instruction AST.
 *       The method AST will destroy it when the method AST is destroyed.
 */
void ar__method_ast__add_instruction(ar_method_ast_t* mut_ast, ar_instruction_ast_t* own_instruction);

/**
 * Get the number of instructions in the method AST.
 * 
 * @param ref_ast The method AST to query (borrowed reference)
 * @return The number of instructions
 */
size_t ar__method_ast__get_instruction_count(const ar_method_ast_t* ref_ast);

#endif /* AGERUN_METHOD_AST_H */