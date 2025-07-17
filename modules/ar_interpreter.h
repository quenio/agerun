#ifndef AGERUN_INTERPRETER_H
#define AGERUN_INTERPRETER_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_data.h"
#include "ar_method.h"

/* Interpreter instance (opaque type) */
typedef struct ar_interpreter_s ar_interpreter_t;

/**
 * Creates a new interpreter instance
 * @return Newly created interpreter, or NULL on failure
 * @note Ownership: Returns an owned object that the caller must destroy with ar_interpreter__destroy.
 */
ar_interpreter_t* ar_interpreter__create(void);

/**
 * Destroys an interpreter instance and frees its resources
 * @param own_interpreter The interpreter to destroy (owned reference, will be freed)
 * @note Ownership: This function takes ownership of the interpreter and frees it.
 *       The pointer will be invalid after this call.
 */
void ar_interpreter__destroy(ar_interpreter_t *own_interpreter);


/**
 * Executes a method in the context of an agent
 * @param mut_interpreter The interpreter instance (mutable reference)
 * @param agent_id The ID of the agent executing the method
 * @param ref_message The message being processed (borrowed reference)
 * @return true if execution was successful, false otherwise
 * @note Ownership: Function does not take ownership of any parameters.
 *       This function retrieves the agent's method, creates an instruction context,
 *       executes the method's instructions line by line, and cleans up the context.
 */
bool ar_interpreter__execute_method(ar_interpreter_t *mut_interpreter,
                                    int64_t agent_id, 
                                    const ar_data_t *ref_message);

#endif /* AGERUN_INTERPRETER_H */
