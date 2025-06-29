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

#endif /* AGERUN_LOG_H */