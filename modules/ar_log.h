/**
 * @file ar_log.h
 * @brief Log module for managing event collections
 * 
 * The log module provides a buffered event logging system with automatic
 * disk persistence. Events are buffered in memory and flushed to disk when
 * the buffer is full or when the log is destroyed.
 */

#ifndef AGERUN_LOG_H
#define AGERUN_LOG_H

#include "ar_event.h"

/**
 * Opaque type for a log instance
 */
typedef struct ar_log_s ar_log_t;

/**
 * Creates a new log instance
 * @return A new log instance, or NULL on failure
 * @note Ownership: Returns an owned value that caller must destroy.
 *       Creates/appends to "agerun.log" file in current directory.
 */
ar_log_t* ar_log__create(void);

/**
 * Destroys a log instance and flushes any buffered events
 * @param own_log The log to destroy
 * @note Ownership: Takes ownership and destroys the log.
 *       Flushes any buffered events to disk before destroying.
 */
void ar_log__destroy(ar_log_t *own_log);

/**
 * Logs an error message
 * @param mut_log The log instance (mutable reference)
 * @param message The error message to log
 * @note The message is copied internally. Events are buffered until
 *       the buffer is full or the log is destroyed.
 */
void ar_log__error(ar_log_t *mut_log, const char *message);

/**
 * Logs a warning message
 * @param mut_log The log instance (mutable reference)
 * @param message The warning message to log
 * @note The message is copied internally. Events are buffered until
 *       the buffer is full or the log is destroyed.
 */
void ar_log__warning(ar_log_t *mut_log, const char *message);

/**
 * Logs an info message
 * @param mut_log The log instance (mutable reference)
 * @param message The info message to log
 * @note The message is copied internally. Events are buffered until
 *       the buffer is full or the log is destroyed.
 */
void ar_log__info(ar_log_t *mut_log, const char *message);

/**
 * Logs an error message with position
 * @param mut_log The log instance (mutable reference)
 * @param message The error message to log
 * @param position The position where the error occurred
 * @note The message is copied internally. Events are buffered until
 *       the buffer is full or the log is destroyed.
 */
void ar_log__error_at(ar_log_t *mut_log, const char *message, int position);

/**
 * Logs a warning message with position
 * @param mut_log The log instance (mutable reference)
 * @param message The warning message to log
 * @param position The position where the warning occurred
 * @note The message is copied internally. Events are buffered until
 *       the buffer is full or the log is destroyed.
 */
void ar_log__warning_at(ar_log_t *mut_log, const char *message, int position);

/**
 * Logs an info message with position
 * @param mut_log The log instance (mutable reference)
 * @param message The info message to log
 * @param position The position where the info occurred
 * @note The message is copied internally. Events are buffered until
 *       the buffer is full or the log is destroyed.
 */
void ar_log__info_at(ar_log_t *mut_log, const char *message, int position);

/**
 * Gets the last error event from the log
 * @param ref_log The log instance (borrowed reference)
 * @return The last error event, or NULL if no errors exist
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
ar_event_t* ar_log__get_last_error(ar_log_t *ref_log);

/**
 * Gets the last warning event from the log
 * @param ref_log The log instance (borrowed reference)
 * @return The last warning event, or NULL if no warnings exist
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
ar_event_t* ar_log__get_last_warning(ar_log_t *ref_log);

/**
 * Gets the last info event from the log
 * @param ref_log The log instance (borrowed reference)
 * @return The last info event, or NULL if no info events exist
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 */
ar_event_t* ar_log__get_last_info(ar_log_t *ref_log);

/**
 * Gets the last error message from the log
 * @param ref_log The log instance (borrowed reference)
 * @return The error message, or NULL if no errors exist
 * @note Ownership: Returns a borrowed reference. Do not destroy.
 *       This is a convenience function that combines ar_log__get_last_error
 *       and ar_event__get_message.
 */
const char* ar_log__get_last_error_message(ar_log_t *ref_log);

/**
 * Gets the position of the last error from the log
 * @param ref_log The log instance (borrowed reference)
 * @return The error position, or 0 if no errors exist or position not set
 * @note This is a convenience function that combines ar_log__get_last_error,
 *       ar_event__has_position, and ar_event__get_position.
 */
int ar_log__get_last_error_position(ar_log_t *ref_log);

#endif /* AGERUN_LOG_H */
