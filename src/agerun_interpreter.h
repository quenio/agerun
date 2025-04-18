#ifndef AGERUN_INTERPRETER_H
#define AGERUN_INTERPRETER_H

#include <stdbool.h>
#include "agerun_data.h"

// Forward declarations
struct agent_s;

/**
 * Parse and execute a single instruction
 * @param agent The agent executing the instruction
 * @param message The message being processed
 * @param instruction The instruction to execute
 * @return true if execution was successful, false otherwise
 */
bool ar_instruction_run(struct agent_s *agent, const char *message, const char *instruction);

/**
 * Evaluate an expression in the agent's context
 * @param agent The agent context
 * @param message The message being processed
 * @param expr The expression to evaluate
 * @param offset Pointer to current position in the expression
 * @return The evaluated data result
 */
data_t ar_evaluate_expression(struct agent_s *agent, const char *message, const char *expr, int *offset);

/**
 * Interprets and executes a method's instructions in the context of an agent
 * @param agent The agent executing the method
 * @param message The message being processed
 * @param instructions The method instructions to execute
 * @return true if execution was successful, false otherwise
 */
bool ar_interpret_agent_method(struct agent_s *agent, const char *message, const char *instructions);

#endif /* AGERUN_INTERPRETER_H */
