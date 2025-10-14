/**
 * @file ar_delegate.h
 * @brief Delegate module for managing external communication
 *
 * The delegate module provides a foundation for mediating between agents
 * and external resources (files, network, logging, etc.) with built-in
 * security controls and sandboxing.
 */

#ifndef AGERUN_DELEGATE_H
#define AGERUN_DELEGATE_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_log.h"
#include "ar_data.h"

/**
 * Opaque type for a delegate instance
 */
typedef struct ar_delegate_s ar_delegate_t;

/**
 * Creates a new delegate instance
 * @param ref_log The log instance for error reporting (borrowed reference)
 * @param type The delegate type identifier (borrowed string, e.g., "file", "network", "log")
 * @return A new delegate instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       The delegate borrows the log reference - caller must ensure log outlives delegate.
 *       The delegate borrows the type string - caller must ensure string outlives delegate.
 */
ar_delegate_t* ar_delegate__create(ar_log_t *ref_log, const char *type);

/**
 * Destroys a delegate instance
 * @param own_delegate The delegate to destroy
 * @note Ownership: Takes ownership and destroys the delegate.
 */
void ar_delegate__destroy(ar_delegate_t *own_delegate);

/**
 * Gets the log instance from a delegate
 * @param ref_delegate The delegate instance
 * @return The log instance, or NULL if not set
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
ar_log_t* ar_delegate__get_log(const ar_delegate_t *ref_delegate);

/**
 * Gets the type identifier from a delegate
 * @param ref_delegate The delegate instance
 * @return The type identifier, or NULL if not set
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
const char* ar_delegate__get_type(const ar_delegate_t *ref_delegate);

/**
 * Handles a message sent to this delegate
 * @param ref_delegate The delegate instance
 * @param ref_message The message to handle (borrowed reference)
 * @param sender_id The ID of the agent sending the message
 * @return true if message was handled, false otherwise
 * @note Ownership: Delegate borrows the message, does not take ownership.
 *       Caller retains ownership and must destroy the message.
 */
bool ar_delegate__handle_message(ar_delegate_t *ref_delegate, ar_data_t *ref_message, int64_t sender_id);

/**
 * Sends a message to this delegate's queue
 * @param mut_delegate The delegate instance to receive the message
 * @param own_message The message to send (takes ownership)
 * @return true if message was queued successfully, false otherwise
 * @note Ownership: Takes ownership of the message. Message will be destroyed
 *       by delegate if send fails, or when queue is destroyed.
 */
bool ar_delegate__send(ar_delegate_t *mut_delegate, ar_data_t *own_message);

/**
 * Checks if this delegate has any queued messages
 * @param ref_delegate The delegate instance to check
 * @return true if delegate has queued messages, false otherwise
 */
bool ar_delegate__has_messages(const ar_delegate_t *ref_delegate);

/**
 * Takes the next message from this delegate's queue
 * @param mut_delegate The delegate instance
 * @return The next queued message, or NULL if queue is empty
 * @note Ownership: Returns an owned value. Caller MUST destroy the returned message.
 *       Delegate drops ownership when returning the message.
 */
ar_data_t* ar_delegate__take_message(ar_delegate_t *mut_delegate);

#endif /* AGERUN_DELEGATE_H */
