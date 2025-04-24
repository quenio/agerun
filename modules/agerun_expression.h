#ifndef AGERUN_EXPRESSION_H
#define AGERUN_EXPRESSION_H

#include <stdbool.h>
#include "agerun_data.h"
#include "agerun_agent.h"

/**
 * Context structure for expression evaluation.
 * Contains all the information needed during expression parsing and evaluation.
 * 
 * This is an opaque type. Clients should use the provided functions to create,
 * manipulate, and destroy expression contexts.
 */
typedef struct expr_context_s expr_context_t;

/**
 * Creates a new expression evaluation context.
 *
 * @param agent The agent context (can be NULL for standalone evaluation)
 * @param message The message being processed (can be NULL if not needed)
 * @param expr The expression string to evaluate
 * @return Newly created expression context, or NULL on failure
 */
expr_context_t* ar_expression_create_context(agent_t *agent, data_t *message, const char *expr);

/**
 * Destroys an expression context and frees all associated resources.
 *
 * @param ctx The expression context to destroy
 */
void ar_expression_destroy_context(expr_context_t *ctx);

/**
 * Gets the current parsing offset in the expression string.
 *
 * @param ctx The expression context
 * @return Current offset in the expression string
 */
int ar_expression_offset(const expr_context_t *ctx);

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
data_t* ar_expression_evaluate(expr_context_t *ctx);

#endif /* AGERUN_EXPRESSION_H */
