#ifndef AGERUN_SYSTEM_H
#define AGERUN_SYSTEM_H

#include <stdbool.h>
#include "agerun_agent.h"

/**
 * Initialize the Agerun runtime system
 * @param ref_method_name Name of the initial method to run (borrowed reference)
 * @param version Version of the method (0 for latest)
 * @return ID of the created initial agent, or 0 on failure
 * @note Ownership: Function copies the method name; does not take ownership of ref_method_name.
 */
agent_id_t ar_system_init(const char *ref_method_name, version_t version);

/**
 * Shut down the Agerun runtime system
 * @note Ownership: Frees all resources owned by the system.
 */
void ar_system_shutdown(void);

/**
 * Process the next pending message in the system
 * @return true if a message was processed, false if no messages
 * @note Ownership: Takes ownership of and destroys the processed message.
 */
bool ar_system_process_next_message(void);

/**
 * Process all pending messages in the system
 * @return Number of messages processed
 * @note Ownership: Takes ownership of and destroys all processed messages.
 */
int ar_system_process_all_messages(void);

#endif /* AGERUN_SYSTEM_H */
