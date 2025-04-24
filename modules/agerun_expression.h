#ifndef AGERUN_EXPRESSION_H
#define AGERUN_EXPRESSION_H

#include <stdbool.h>
#include "agerun_data.h"
#include "agerun_agent.h"

/**
 * Context structure for expression evaluation.
 * Contains all the information needed during expression parsing and evaluation.
 */
typedef struct {
    agent_t *agent;     /* The agent context */
    data_t *message;    /* The message being processed */
    const char *expr;   /* The expression to evaluate */
    int offset;         /* Current position in the expression */
} expr_context_t;

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
 * @param ctx Pointer to the expression evaluation context containing agent, message, expression string, and offset
 * @return Pointer to the evaluated data result, or NULL on failure
 */
data_t* ar_expression_evaluate(expr_context_t *ctx);

#endif /* AGERUN_EXPRESSION_H */
