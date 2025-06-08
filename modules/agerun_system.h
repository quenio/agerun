#ifndef AGERUN_SYSTEM_H
#define AGERUN_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Initialize the Agerun runtime system
 * @param ref_method_name Name of the initial method to run (borrowed reference)
 * @param ref_version Version string of the method (NULL for latest)
 * @return ID of the created initial agent, or 0 on failure
 * @note Ownership: Function copies the method name; does not take ownership of ref_method_name.
 */
int64_t ar__system__init(const char *ref_method_name, const char *ref_version);

/**
 * Shut down the Agerun runtime system
 * @note Ownership: Frees all resources owned by the system.
 */
void ar__system__shutdown(void);

/**
 * Process the next pending message in the system
 * @return true if a message was processed, false if no messages
 * @note Ownership: Takes ownership of and destroys the processed message.
 */
bool ar__system__process_next_message(void);

/**
 * Process all pending messages in the system
 * @return Number of messages processed
 * @note Ownership: Takes ownership of and destroys all processed messages.
 */
int ar__system__process_all_messages(void);

#endif /* AGERUN_SYSTEM_H */
