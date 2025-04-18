#ifndef AGERUN_SYSTEM_H
#define AGERUN_SYSTEM_H

#include <stdbool.h>
#include "agerun_agent.h"

/**
 * Initialize the Agerun runtime system
 * @param method_name Name of the initial method to run
 * @param version Version of the method (0 for latest)
 * @return ID of the created initial agent, or 0 on failure
 */
agent_id_t ar_init(const char *method_name, version_t version);

/**
 * Shut down the Agerun runtime system
 */
void ar_shutdown(void);

/**
 * Process the next pending message in the system
 * @return true if a message was processed, false if no messages
 */
bool ar_process_next_message(void);

/**
 * Process all pending messages in the system
 * @return Number of messages processed
 */
int ar_process_all_messages(void);

#endif /* AGERUN_SYSTEM_H */
