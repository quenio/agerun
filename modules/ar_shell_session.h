#ifndef AGERUN_SHELL_SESSION_H
#define AGERUN_SHELL_SESSION_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_data.h"

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

#endif /* AGERUN_SHELL_SESSION_H */
