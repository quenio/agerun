#ifndef AGERUN_METHOD_H
#define AGERUN_METHOD_H

#include <stdbool.h>
#include "agerun_data.h"
#include "agerun_agent.h"

/**
 * Interprets and executes a method's instructions in the context of an agent
 * @param agent The agent executing the method
 * @param message The message being processed
 * @param instructions The method instructions to execute
 * @return true if execution was successful, false otherwise
 */
bool ar_method_run(agent_t *agent, const char *message, const char *instructions);

#endif /* AGERUN_METHOD_H */
