#ifndef AGERUN_INSTRUCTION_H
#define AGERUN_INSTRUCTION_H

#include <stdbool.h>
#include "agerun_data.h"
#include "agerun_agent.h"

/**
 * Parse and execute a single instruction
 * @param agent The agent executing the instruction
 * @param message The message being processed (can be any data type)
 * @param instruction The instruction to execute
 * @return true if execution was successful, false otherwise
 */
bool ar_instruction_run(agent_t *agent, const data_t *message, const char *instruction);

#endif /* AGERUN_INSTRUCTION_H */
