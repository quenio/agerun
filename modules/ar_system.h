#ifndef AGERUN_SYSTEM_H
#define AGERUN_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_agency.h"

/* Opaque type declaration */
typedef struct ar_system_s ar_system_t;

/**
 * Create a new system instance
 * @param ref_agency The agency instance to use (borrowed reference), or NULL for global agency
 * @return New system instance (ownership transferred), or NULL on failure
 * @note Ownership: Caller must destroy the returned system.
 */
ar_system_t* ar_system__create(ar_agency_t *ref_agency);

/**
 * Destroy a system instance
 * @param own_system The system to destroy (ownership transferred)
 * @note Ownership: Destroys all system resources and the system itself.
 */
void ar_system__destroy(ar_system_t *own_system);

/**
 * Initialize a system instance
 * @param mut_system The system instance to initialize (mutable reference)
 * @param ref_method_name Name of the initial method to run (borrowed reference)
 * @param ref_version Version string of the method (NULL for latest)
 * @return ID of the created initial agent, or 0 on failure
 * @note Ownership: Function copies the method name; does not take ownership of ref_method_name.
 */
int64_t ar_system__init_with_instance(ar_system_t *mut_system, const char *ref_method_name, const char *ref_version);

/**
 * Shut down a system instance
 * @param mut_system The system instance to shut down (mutable reference)
 * @note Ownership: Saves persistent data but does not destroy the system instance itself.
 */
void ar_system__shutdown_with_instance(ar_system_t *mut_system);

/**
 * Process the next pending message in the system instance
 * @param mut_system The system instance (mutable reference)
 * @return true if a message was processed, false if no messages
 * @note Ownership: Takes ownership of and destroys the processed message.
 */
bool ar_system__process_next_message_with_instance(ar_system_t *mut_system);

/**
 * Process all pending messages in the system instance
 * @param mut_system The system instance (mutable reference)
 * @return Number of messages processed
 * @note Ownership: Takes ownership of and destroys all processed messages.
 */
int ar_system__process_all_messages_with_instance(ar_system_t *mut_system);

/**
 * Initialize the Agerun runtime system
 * @param ref_method_name Name of the initial method to run (borrowed reference)
 * @param ref_version Version string of the method (NULL for latest)
 * @return ID of the created initial agent, or 0 on failure
 * @note Ownership: Function copies the method name; does not take ownership of ref_method_name.
 */
int64_t ar_system__init(const char *ref_method_name, const char *ref_version);

/**
 * Shut down the Agerun runtime system
 * @note Ownership: Frees all resources owned by the system.
 */
void ar_system__shutdown(void);

/**
 * Process the next pending message in the system
 * @return true if a message was processed, false if no messages
 * @note Ownership: Takes ownership of and destroys the processed message.
 */
bool ar_system__process_next_message(void);

/**
 * Process all pending messages in the system
 * @return Number of messages processed
 * @note Ownership: Takes ownership of and destroys all processed messages.
 */
int ar_system__process_all_messages(void);

#endif /* AGERUN_SYSTEM_H */
