#ifndef AGERUN_EXPRESSION_H
#define AGERUN_EXPRESSION_H

#include <stdbool.h>
#include "agerun_data.h"
#include "agerun_agent.h"

/**
 * Evaluate an expression in the agent's context
 * @param agent The agent context
 * @param message The message being processed (can be any data type)
 * @param expr The expression to evaluate
 * @param offset Pointer to current position in the expression
 * @return Pointer to the evaluated data result, or NULL on failure
 */
data_t* ar_expression_evaluate(agent_t *agent, const data_t *message, const char *expr, int *offset);

#endif /* AGERUN_EXPRESSION_H */

