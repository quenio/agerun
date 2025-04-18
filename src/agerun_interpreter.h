#ifndef AGERUN_INTERPRETER_H
#define AGERUN_INTERPRETER_H

#include <stdbool.h>
#include "agerun_data.h"
#include "agerun_agent.h"

/**
 * Parse and execute a single instruction
 * @param agent The agent executing the instruction
 * @param message The message being processed
 * @param instruction The instruction to execute
 * @return true if execution was successful, false otherwise
 */
bool ar_instruction_run(agent_t *agent, const char *message, const char *instruction);

/**
 * Interprets and executes a method's instructions in the context of an agent
 * @param agent The agent executing the method
 * @param message The message being processed
 * @param instructions The method instructions to execute
 * @return true if execution was successful, false otherwise
 */
bool ar_interpret_agent_method(agent_t *agent, const char *message, const char *instructions);

#endif /* AGERUN_INTERPRETER_H */
