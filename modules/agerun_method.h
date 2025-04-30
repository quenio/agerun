#ifndef AGERUN_METHOD_H
#define AGERUN_METHOD_H

#include <stdbool.h>
#include "agerun_data.h"
#include "agerun_agent.h"

/* Method Definition */
typedef struct method_s {
    char name[64]; /* MAX_METHOD_NAME_LENGTH */
    version_t version;
    version_t previous_version;
    bool backward_compatible;
    bool persist;
    char instructions[16384]; /* MAX_INSTRUCTIONS_LENGTH */
} method_t;

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
