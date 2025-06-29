/**
 * @file ar_event.h
 * @brief Event module for logging and error reporting
 * 
 * The event module provides a representation of individual events
 * (errors, warnings, info messages) that can be logged and inspected.
 * Events are immutable once created.
 */

#ifndef AGERUN_EVENT_H
#define AGERUN_EVENT_H

#include <stdbool.h>

/**
 * Opaque type for an event
 */
typedef struct ar_event_s ar_event_t;

/**
 * Event types
 */
typedef enum {
    AR_EVENT_ERROR,
    AR_EVENT_WARNING,
    AR_EVENT_INFO
} ar_event_type_t;

/**
 * Creates a new event
 * @return A new event instance, or NULL on allocation failure
 * @note Ownership: Returns an owned value that caller must destroy.
 */
ar_event_t* ar_event__create(void);

/**
 * Creates a new event with a message
 * @param message The event message
 * @return A new event instance, or NULL on allocation failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The message is copied, so the caller retains ownership of the input.
 */
ar_event_t* ar_event__create_with_message(const char *message);

/**
 * Creates a new event with a type and message
 * @param type The event type
 * @param message The event message
 * @return A new event instance, or NULL on allocation failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The message is copied, so the caller retains ownership of the input.
 */
ar_event_t* ar_event__create_typed(ar_event_type_t type, const char *message);

/**
 * Destroys an event and frees its resources
 * @param own_event The event to destroy
 * @note Ownership: Takes ownership and destroys the event.
 */
void ar_event__destroy(ar_event_t *own_event);

/**
 * Gets the message from an event
 * @param ref_event The event to query
 * @return The event message, or NULL if no message is set
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
const char* ar_event__get_message(const ar_event_t *ref_event);

/**
 * Gets the type of an event
 * @param ref_event The event to query
 * @return The event type
 */
ar_event_type_t ar_event__get_type(const ar_event_t *ref_event);

/**
 * Creates a new event with type, message, and position
 * @param type The event type
 * @param message The event message
 * @param position The position in the source where the event occurred
 * @return A new event instance, or NULL on allocation failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The message is copied, so the caller retains ownership of the input.
 */
ar_event_t* ar_event__create_with_position(ar_event_type_t type, const char *message, int position);

/**
 * Gets the position from an event
 * @param ref_event The event to query
 * @return The position, or -1 if no position is set
 */
int ar_event__get_position(const ar_event_t *ref_event);

/**
 * Checks if an event has position information
 * @param ref_event The event to query
 * @return true if the event has position information, false otherwise
 */
bool ar_event__has_position(const ar_event_t *ref_event);

/**
 * Gets the timestamp from an event
 * @param ref_event The event to query
 * @return The timestamp in ISO 8601 format (YYYY-MM-DDTHH:MM:SS)
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
const char* ar_event__get_timestamp(const ar_event_t *ref_event);

#endif /* AGERUN_EVENT_H */