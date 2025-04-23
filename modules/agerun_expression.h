#ifndef AGERUN_EXPRESSION_H
#define AGERUN_EXPRESSION_H

#include <stdbool.h>
#include "agerun_data.h"
#include "agerun_agent.h"

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
 * @param agent The agent context
 * @param message The message being processed (can be any data type)
 * @param expr The expression to evaluate
 * @param offset Pointer to current position in the expression
 * @return Pointer to the evaluated data result, or NULL on failure
 */
data_t* ar_expression_evaluate(agent_t *agent, data_t *message, const char *expr, int *offset);

#endif /* AGERUN_EXPRESSION_H */
