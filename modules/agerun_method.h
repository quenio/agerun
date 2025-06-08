#ifndef AGERUN_METHOD_H
#define AGERUN_METHOD_H

#include <stdbool.h>
#include <stdint.h>
#include "agerun_data.h"

/* Method Definition (opaque type) */
typedef struct method_s method_t;

/**
 * Creates a new method object with the given parameters
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param ref_version Semantic version string for this method (e.g., "1.0.0")
 * @return Newly created method object, or NULL on failure
 * @note Ownership: Returns an owned object that the caller must destroy with ar_method_destroy.
 *       The method copies the name, instructions, and version. The original strings remain owned by the caller.
 */
method_t* ar__method__create(const char *ref_name, const char *ref_instructions, 
                           const char *ref_version);

/**
 * Get the name of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method name (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar__method__get_name(const method_t *ref_method);

/**
 * Get the version of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method version string (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar__method__get_version(const method_t *ref_method);

// Removed ar_method_is_backward_compatible and ar_method_is_persistent

/**
 * Get the instructions of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method instructions (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar__method__get_instructions(const method_t *ref_method);

/**
 * Destroys a method object and frees its resources
 * @param own_method The method to destroy (owned reference, will be freed)
 * @note Ownership: This function takes ownership of the method and frees it.
 *       The pointer will be invalid after this call.
 */
void ar__method__destroy(method_t *own_method);

/**
 * Interprets and executes a method's instructions in the context of an agent
 * @param agent_id The ID of the agent executing the method
 * @param ref_message The message being processed (borrowed reference, ownership remains with the caller)
 * @param ref_instructions The method instructions to execute (borrowed reference)
 * @return true if execution was successful, false otherwise
 * @note Ownership: Function does not take ownership of any parameters.
 *       The message and instructions remain owned by the caller.
 */
bool ar__method__run(int64_t agent_id, const data_t *ref_message, const char *ref_instructions);

#endif /* AGERUN_METHOD_H */
