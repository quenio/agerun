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
 * @param mut_agent The agent executing the instruction (mutable reference)
 * @param mut_message The message being processed (mutable reference)
 * @param ref_instruction The instruction to execute (borrowed reference)
 * @return true if execution was successful, false otherwise
 * @note Ownership: Does not take ownership of any parameters.
 *       The function does not transfer ownership of any objects.
 */
bool ar_instruction_run(agent_t *mut_agent, data_t *mut_message, const char *ref_instruction);

#endif /* AGERUN_INSTRUCTION_H */
