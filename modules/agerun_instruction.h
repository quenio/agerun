#ifndef AGERUN_INSTRUCTION_H
#define AGERUN_INSTRUCTION_H

#include <stdbool.h>
#include "agerun_data.h"
#include "agerun_agent.h"

/**
 * Parse and execute a single instruction using recursive descent parsing.
 * 
 * The instruction grammar follows the BNF definition:
 * 
 * <instruction> ::= <assignment>
 *                | <function-instruction>
 *                
 * <assignment> ::= <memory-access> ':=' <expression>
 * 
 * <function-instruction> ::= [<memory-access> ':='] <function-call>
 * 
 * <function-call> ::= <send-function>
 *                  | <parse-function>
 *                  | <build-function>
 *                  | <method-function>
 *                  | <agent-function>
 *                  | <destroy-function>
 *                  | <if-function>
 * 
 * <memory-access> ::= 'memory' {'.' <identifier>}
 * 
 * Note: Only 'memory' can be used as the root identifier on the left side of
 * assignments. The ':=' operator is used for all assignments. Memory access
 * uses dot notation (e.g., memory.field.subfield).
 * 
 * @param agent The agent executing the instruction
 * @param message The message being processed (can be any data type)
 * @param instruction The instruction to execute
 * @return true if execution was successful, false otherwise
 */
bool ar_instruction_run(agent_t *agent, const data_t *message, const char *instruction);

#endif /* AGERUN_INSTRUCTION_H */
