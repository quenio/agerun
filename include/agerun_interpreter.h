#ifndef AGERUN_INTERPRETER_H
#define AGERUN_INTERPRETER_H

#include <stdbool.h>

// Forward declarations
struct agent_s;

/**
 * Interprets and executes a method's instructions in the context of an agent
 * @param agent The agent executing the method
 * @param message The message being processed
 * @param instructions The method instructions to execute
 * @return true if execution was successful, false otherwise
 */
bool ar_interpret_agent_method(struct agent_s *agent, const char *message, const char *instructions);

#endif /* AGERUN_INTERPRETER_H */
