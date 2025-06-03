#ifndef AGERUN_INSTRUCTION_H
#define AGERUN_INSTRUCTION_H

#include <stdbool.h>
#include <stdint.h>
#include "agerun_data.h"

/**
 * Context structure for instruction parsing and execution.
 * Contains all the information needed during instruction parsing and execution.
 * 
 * This is an opaque type. Clients should use the provided functions to create,
 * manipulate, and destroy instruction contexts.
 */
typedef struct instruction_context_s instruction_context_t;

/**
 * Creates a new instruction context for parsing and executing instructions.
 *
 * @param mut_memory The memory to use for the instruction (mutable reference, can be NULL if not needed)
 * @param ref_context The context data (borrowed reference, can be NULL if not needed)
 * @param ref_message The message being processed (borrowed reference, can be NULL if not needed)
 * @return Newly created instruction context (owned by caller), or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The function does not take ownership of the memory, context, or message parameters.
 */
instruction_context_t* ar_instruction_create_context(data_t *mut_memory, const data_t *ref_context, const data_t *ref_message);

/**
 * Destroys an instruction context.
 *
 * @param own_ctx The instruction context to destroy (ownership transferred to function)
 * @note Ownership: Takes ownership of the context parameter and destroys it.
 *       This only frees the context structure itself,
 *       not the memory, context, or message data structures which are owned by the caller.
 */
void ar_instruction_destroy_context(instruction_context_t *own_ctx);

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
 * @param mut_ctx The instruction context to use (mutable reference)
 * @param ref_instruction The instruction to execute (borrowed reference)
 * @return true if execution was successful, false otherwise
 * @note Ownership: Does not take ownership of any parameters.
 *       The function does not transfer ownership of any objects.
 */
bool ar_instruction_run(instruction_context_t *mut_ctx, const char *ref_instruction);

/**
 * Gets the memory from the instruction context.
 *
 * @param ref_ctx The instruction context (borrowed reference)
 * @return Mutable reference to the memory (not owned by caller)
 * @note Ownership: Does not take ownership of the context parameter.
 *       The returned memory is still owned by the context.
 */
data_t* ar_instruction_get_memory(const instruction_context_t *ref_ctx);

/**
 * Gets the context data from the instruction context.
 *
 * @param ref_ctx The instruction context (borrowed reference)
 * @return Borrowed reference to the context data (not owned by caller)
 * @note Ownership: Does not take ownership of the context parameter.
 *       The returned context data is still owned by the context owner.
 */
const data_t* ar_instruction_get_context(const instruction_context_t *ref_ctx);

/**
 * Gets the message from the instruction context.
 *
 * @param ref_ctx The instruction context (borrowed reference)
 * @return Borrowed reference to the message (not owned by caller)
 * @note Ownership: Does not take ownership of the context parameter.
 *       The returned message is still owned by the context owner.
 */
const data_t* ar_instruction_get_message(const instruction_context_t *ref_ctx);

/**
 * Send a message to another agent.
 *
 * @param target_id The ID of the agent to send to
 * @param own_message The message to send (ownership transferred)
 * @return true if sending was successful, false otherwise
 * @note Ownership: Takes ownership of own_message.
 *       If sending fails, the function will destroy the message.
 */
bool ar_instruction_send_message(int64_t target_id, data_t *own_message);

/**
 * Gets the last error message from the instruction context.
 *
 * @param ref_ctx The instruction context (borrowed reference)
 * @return The last error message, or NULL if no error
 * @note Ownership: Does not take ownership of the context parameter.
 *       The returned string is owned by the context and should not be freed.
 */
const char* ar_instruction_get_last_error(const instruction_context_t *ref_ctx);

/**
 * Gets the position in the instruction string where the last error occurred.
 *
 * @param ref_ctx The instruction context (borrowed reference)
 * @return The error position (1-based column), or 0 if no error
 * @note Ownership: Does not take ownership of the context parameter.
 */
int ar_instruction_get_error_position(const instruction_context_t *ref_ctx);

#endif /* AGERUN_INSTRUCTION_H */
