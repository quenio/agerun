#ifndef AGERUN_EXPRESSION_H
#define AGERUN_EXPRESSION_H

#include <stdbool.h>
#include "agerun_data.h"

/**
 * Context structure for expression evaluation.
 * Contains all the information needed during expression parsing and evaluation.
 * 
 * This is an opaque type. Clients should use the provided functions to create,
 * manipulate, and destroy expression contexts.
 */
typedef struct expression_context_s expression_context_t;

/**
 * Creates a new expression evaluation context.
 *
 * @param memory The agent's memory data (can be NULL if not needed)
 * @param context The agent's context data (can be NULL if not needed)
 * @param message The message being processed (can be NULL if not needed)
 * @param expr The expression string to evaluate
 * @return Newly created expression context, or NULL on failure
 */
expression_context_t* ar_expression_create_context(data_t *memory, data_t *context, data_t *message, const char *expr);

/**
 * Destroys an expression context.
 * Note: This only frees the context structure itself, not the memory, context, or message
 * data structures which are owned by the caller.
 *
 * @param ctx The expression context to destroy
 */
void ar_expression_destroy_context(expression_context_t *ctx);

/**
 * Gets the current parsing offset in the expression string.
 *
 * @param ctx The expression context
 * @return Current offset in the expression string
 */
int ar_expression_offset(const expression_context_t *ctx);

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
 * @param ctx Pointer to the expression evaluation context
 * @return Pointer to the evaluated data result, or NULL on failure
 */
data_t* ar_expression_evaluate(expression_context_t *ctx);

/**
 * Take ownership of a result from the expression context.
 * 
 * This function removes the result from the context's tracked results list,
 * so it won't be destroyed when the context is destroyed. The caller
 * becomes responsible for destroying the result when no longer needed.
 *
 * @param ctx Pointer to the expression evaluation context
 * @param result The result to take ownership of
 * @return true if ownership was successfully transferred, false otherwise
 */
bool ar_expression_take_ownership(expression_context_t *ctx, data_t *result);

#endif /* AGERUN_EXPRESSION_H */
