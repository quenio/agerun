#ifndef AGERUN_METHOD_H
#define AGERUN_METHOD_H

#include <stdbool.h>
#include "agerun_data.h"
#include "agerun_agent.h"

/* Method Definition (opaque type) */
typedef struct method_s method_t;

/**
 * Creates a new method object with the given parameters
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param version The version number for this method
 * @param previous_version Previous version number (0 for first version)
 * @param backward_compatible Whether the method is backward compatible
 * @param persist Whether agents using this method should persist
 * @return Newly created method object, or NULL on failure
 * @note Ownership: Returns an owned object that the caller must destroy with ar_method_destroy.
 *       The method copies the name and instructions. The original strings remain owned by the caller.
 */
method_t* ar_method_create_object(const char *ref_name, const char *ref_instructions, 
                         version_t version, version_t previous_version, 
                         bool backward_compatible, bool persist);

/**
 * Define a new method with the given instructions
 * @param ref_name Method name (borrowed reference)
 * @param ref_instructions The method implementation code (borrowed reference)
 * @param previous_version Previous version number (0 for new method)
 * @param backward_compatible Whether the method is backward compatible
 * @param persist Whether agents using this method should persist
 * @return New version number, or 0 on failure
 * @note Ownership: The method copies the name and instructions. The original strings
 *       remain owned by the caller.
 */
version_t ar_method_create(const char *ref_name, const char *ref_instructions, 
                        version_t previous_version, bool backward_compatible, 
                        bool persist);

/**
 * Get the name of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method name (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar_method_get_name(const method_t *ref_method);

/**
 * Get the version of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method version
 */
version_t ar_method_get_version(const method_t *ref_method);

/**
 * Get the previous version of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Previous method version (0 if this is the first version)
 */
version_t ar_method_get_previous_version(const method_t *ref_method);

/**
 * Check if a method is backward compatible
 * @param ref_method Method reference (borrowed reference)
 * @return true if the method is backward compatible, false otherwise
 */
bool ar_method_is_backward_compatible(const method_t *ref_method);

/**
 * Check if a method is persistent
 * @param ref_method Method reference (borrowed reference)
 * @return true if the method is persistent, false otherwise
 */
bool ar_method_is_persistent(const method_t *ref_method);

/**
 * Get the instructions of a method
 * @param ref_method Method reference (borrowed reference)
 * @return Method instructions (borrowed reference)
 * @note Ownership: Returns a borrowed reference. The caller should not free the result.
 */
const char* ar_method_get_instructions(const method_t *ref_method);

/**
 * Destroys a method object and frees its resources
 * @param own_method The method to destroy (owned reference, will be freed)
 * @note Ownership: This function takes ownership of the method and frees it.
 *       The pointer will be invalid after this call.
 */
void ar_method_destroy(method_t *own_method);

/**
 * Interprets and executes a method's instructions in the context of an agent
 * @param mut_agent The agent executing the method (mutable reference)
 * @param mut_message The message being processed (mutable reference, ownership remains with the caller)
 * @param ref_instructions The method instructions to execute (borrowed reference)
 * @return true if execution was successful, false otherwise
 * @note Ownership: Function does not take ownership of any parameters.
 *       The agent, message, and instructions remain owned by the caller.
 */
bool ar_method_run(agent_t *mut_agent, data_t *mut_message, const char *ref_instructions);

#endif /* AGERUN_METHOD_H */
