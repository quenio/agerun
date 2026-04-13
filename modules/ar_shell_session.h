#ifndef AGERUN_SHELL_SESSION_H
#define AGERUN_SHELL_SESSION_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_data.h"
#include "ar_delegate.h"
#include "ar_system.h"

/**
 * Shell acknowledgement mode.
 */
typedef enum {
    AR_SHELL_MODE__NORMAL,
    AR_SHELL_MODE__VERBOSE
} ar_shell_mode_t;

/**
 * Opaque shell-session type.
 */
typedef struct ar_shell_session_s ar_shell_session_t;

/**
 * Create a shell session.
 * @param session_id Value identifier for the session
 * @param mode Startup acknowledgement mode
 * @return Owned shell session, or NULL on failure
 */
ar_shell_session_t* ar_shell_session__create(int64_t session_id, ar_shell_mode_t mode);

/**
 * Destroy a shell session.
 * @param own_session Owned shell session
 */
void ar_shell_session__destroy(ar_shell_session_t *own_session);

/**
 * Get the session identifier.
 * @param ref_session Borrowed shell session
 * @return Session identifier, or 0 if ref_session is NULL
 */
int64_t ar_shell_session__get_session_id(const ar_shell_session_t *ref_session);

/**
 * Get the acknowledgement mode.
 * @param ref_session Borrowed shell session
 * @return Session mode
 */
ar_shell_mode_t ar_shell_session__get_mode(const ar_shell_session_t *ref_session);

/**
 * Get the receiving-agent identifier.
 * @param ref_session Borrowed shell session
 * @return Agent identifier, or 0 if no receiving agent is active
 */
int64_t ar_shell_session__get_agent_id(const ar_shell_session_t *ref_session);

/**
 * Activate the session with a receiving agent.
 * @param mut_session Mutable shell session
 * @param agent_id Receiving-agent identifier
 * @return true on success, false otherwise
 */
bool ar_shell_session__activate(ar_shell_session_t *mut_session, int64_t agent_id);

/**
 * Get the runtime delegate ID used for shell-session mediation.
 * @param ref_session Borrowed shell session
 * @return Negative delegate ID, or 0 if unavailable
 */
int64_t ar_shell_session__get_runtime_delegate_id(const ar_shell_session_t *ref_session);

/**
 * Create the runtime delegate that mediates shell-session protocol messages.
 * @param ref_session Borrowed shell session
 * @param ref_system Borrowed system reference
 * @return Owned delegate, or NULL on failure
 */
ar_delegate_t* ar_shell_session__create_runtime_delegate(
    ar_shell_session_t *mut_session,
    ar_system_t *mut_system);

/**
 * Store one value into the shell-session memory map.
 * @param mut_session Mutable shell session
 * @param ref_path Borrowed session path rooted at memory
 * @param own_value Owned value to store
 * @return true on success, false otherwise
 * @note Ownership: Takes ownership of own_value only on success
 */
bool ar_shell_session__store_value(
    ar_shell_session_t *mut_session,
    const char *ref_path,
    ar_data_t *own_value);

/**
 * Load one value from the shell-session memory map and build a protocol reply.
 * @param ref_session Borrowed shell session
 * @param ref_path Borrowed session path rooted at memory
 * @param request_id Correlation identifier
 * @return Owned reply map, or NULL on failure
 */
ar_data_t* ar_shell_session__load_value(
    const ar_shell_session_t *ref_session,
    const char *ref_path,
    int request_id);

/**
 * Build a loaded-value reply map.
 * @param request_id Correlation identifier
 * @param own_value Owned loaded value
 * @return Owned reply map, or NULL on failure
 * @note Ownership: Takes ownership of own_value on success; caller must destroy on failure
 */
ar_data_t* ar_shell_session__return_loaded_value(int request_id, ar_data_t *own_value);

/**
 * Build an operation-failure reply map.
 * @param request_id Correlation identifier
 * @param ref_reason Borrowed failure reason string
 * @return Owned reply map, or NULL on failure
 */
ar_data_t* ar_shell_session__report_operation_failure(int request_id, const char *ref_reason);

/**
 * Check whether the session is active.
 * @param ref_session Borrowed shell session
 * @return true when active, false otherwise
 */
bool ar_shell_session__is_active(const ar_shell_session_t *ref_session);

/**
 * Get the owned shell-session memory map.
 * @param ref_session Borrowed shell session
 * @return Borrowed memory map reference, or NULL if unavailable
 */
ar_data_t* ar_shell_session__get_memory(const ar_shell_session_t *ref_session);

/**
 * Get the owned shell-session context map used for runtime execution.
 * @param ref_session Borrowed shell session
 * @return Borrowed context map reference, or NULL if unavailable
 */
ar_data_t* ar_shell_session__get_context(const ar_shell_session_t *ref_session);

#endif /* AGERUN_SHELL_SESSION_H */
