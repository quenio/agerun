#ifndef AGERUN_EXPRESSION_H
#define AGERUN_EXPRESSION_H

#include <stdbool.h>
#include "ar_data.h"

/**
 * Context structure for expression evaluation.
 * Contains all the information needed during expression parsing and evaluation.
 * 
 * This is an opaque type. Clients should use the provided functions to create,
 * manipulate, and destroy expression contexts.
 */
typedef struct expression_context_s ar_expression_context_t;

/**
 * Creates a new expression evaluation context.
 *
 * @param mut_memory The agent's memory data (mutable reference, can be NULL if not needed)
 * @param ref_context The agent's context data (borrowed reference, can be NULL if not needed)
 * @param ref_message The message being processed (borrowed reference, can be NULL if not needed)
 * @param ref_expr The expression string to evaluate (borrowed reference)
 * @return Newly created expression context (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The function does not take ownership of the memory, context, or message parameters.
 */
ar_expression_context_t* ar_expression__create_context(ar_data_t *mut_memory, const ar_data_t *ref_context, const ar_data_t *ref_message, const char *ref_expr);

/**
 * Destroys an expression context.
 *
 * @param own_ctx The expression context to destroy (ownership transferred to function)
 * @note Ownership: Takes ownership of the context parameter and destroys it.
 *       This only frees the context structure itself and the results it owns,
 *       not the memory, context, or message data structures which are owned by the caller.
 */
void ar_expression__destroy_context(ar_expression_context_t *own_ctx);

/**
 * Gets the current parsing offset in the expression string.
 *
 * @param ref_ctx The expression context (borrowed reference)
 * @return Current offset in the expression string
 * @note Ownership: Does not take ownership of the context parameter.
 */
int ar_expression__offset(const ar_expression_context_t *ref_ctx);

/**
 * Evaluate an expression in the agent's context using recursive descent parsing.
 * 
 * The expression grammar follows the BNF definition:
 *
 * <expression> ::= <string-literal>
 *                | <number-literal>
 *                | <memory-access>
 *                | <arithmetic-expression>
 *                | <comparison-expression>
 *
 * <string-literal> ::= '"' <characters> '"'
 *
 * <number-literal> ::= <integer>
 *                    | <double>
 *
 * <integer> ::= ['-'] <digit> {<digit>}
 * <double>  ::= <integer> '.' <digit> {<digit>}
 *
 * <memory-access> ::= 'message' {'.' <identifier>}
 *                   | 'memory' {'.' <identifier>}
 *                   | 'context' {'.' <identifier>}
 *
 * <arithmetic-expression> ::= <expression> <arithmetic-operator> <expression>
 * <arithmetic-operator> ::= '+' | '-' | '*' | '/'
 *
 * <comparison-expression> ::= <expression> <comparison-operator> <expression>
 * <comparison-operator> ::= '=' | '<>' | '<' | '<=' | '>' | '>='
 * 
 * @param mut_ctx Pointer to the expression evaluation context (mutable reference)
 * @return Pointer to the evaluated data result, or NULL on failure
 * @note Ownership: Returns a reference managed by the context. Memory access expressions
 *       return direct references to existing data. Other expression types create new objects
 *       that will be destroyed when the context is destroyed unless ownership is transferred
 *       using ar_expression_take_ownership().
 *       The return value may be const or non-const depending on the type of expression.
 */
const ar_data_t* ar_expression__evaluate(ar_expression_context_t *mut_ctx);

/**
 * Take ownership of a result from the expression context.
 * 
 * This function removes the result from the context's tracked results list,
 * so it won't be destroyed when the context is destroyed. The caller
 * becomes responsible for destroying the result when no longer needed.
 *
 * @param mut_ctx Pointer to the expression evaluation context (mutable reference)
 * @param ref_result The result to take ownership of (becomes owned by caller)
 * @return A non-const pointer to the result if ownership was successfully transferred,
 *         NULL otherwise.
 * @note Ownership: Transfers ownership of result from context to caller.
 *       After a successful call, the caller becomes responsible for eventually
 *       destroying the returned pointer with ar_data_destroy().
 *       The const qualifier is removed from ref_result after ownership transfer,
 *       and the returned pointer can be used with functions that take ownership.
 */
ar_data_t* ar_expression__take_ownership(ar_expression_context_t *mut_ctx, const ar_data_t *ref_result);

#endif /* AGERUN_EXPRESSION_H */
