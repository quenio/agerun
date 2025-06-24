#ifndef AGERUN_INTERPRETER_H
#define AGERUN_INTERPRETER_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_data.h"
#include "ar_instruction.h"
#include "ar_method.h"

/* Interpreter instance (opaque type) */
typedef struct interpreter_s interpreter_t;

/**
 * Creates a new interpreter instance
 * @return Newly created interpreter, or NULL on failure
 * @note Ownership: Returns an owned object that the caller must destroy with ar__interpreter__destroy.
 */
interpreter_t* ar__interpreter__create(void);

/**
 * Destroys an interpreter instance and frees its resources
 * @param own_interpreter The interpreter to destroy (owned reference, will be freed)
 * @note Ownership: This function takes ownership of the interpreter and frees it.
 *       The pointer will be invalid after this call.
 */
void ar__interpreter__destroy(interpreter_t *own_interpreter);

/**
 * Executes a single instruction in the given context
 * @param mut_interpreter The interpreter instance (mutable reference)
 * @param mut_context The instruction context containing agent memory, context, and message (mutable reference)
 * @param ref_instruction The instruction string to execute (borrowed reference)
 * @return true if execution was successful, false otherwise
 * @note Ownership: Function does not take ownership of any parameters.
 *       The context may be modified during execution.
 *       This function will contain the core logic moved from ar__instruction__run.
 */
bool ar__interpreter__execute_instruction(interpreter_t *mut_interpreter, 
                                         instruction_context_t *mut_context, 
                                         const char *ref_instruction);

/**
 * Executes a method in the context of an agent
 * @param mut_interpreter The interpreter instance (mutable reference)
 * @param agent_id The ID of the agent executing the method
 * @param ref_message The message being processed (borrowed reference)
 * @param ref_method The method to execute (borrowed reference)
 * @return true if execution was successful, false otherwise
 * @note Ownership: Function does not take ownership of any parameters.
 *       This function creates an instruction context, executes the method's
 *       instructions line by line, and cleans up the context.
 */
bool ar__interpreter__execute_method(interpreter_t *mut_interpreter,
                                    int64_t agent_id, 
                                    const data_t *ref_message, 
                                    const method_t *ref_method);

#endif /* AGERUN_INTERPRETER_H */
