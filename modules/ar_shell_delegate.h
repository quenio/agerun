#ifndef AGERUN_SHELL_DELEGATE_H
#define AGERUN_SHELL_DELEGATE_H

#include <stdbool.h>
#include <stdint.h>
#include "ar_data.h"
#include "ar_log.h"
#include "ar_shell_session.h"
#include "ar_system.h"

/**
 * Opaque shell-delegate type.
 */
typedef struct ar_shell_delegate_s ar_shell_delegate_t;

/**
 * Create a session-specific shell delegate.
 * @param ref_log Borrowed log reference
 * @param ref_session Borrowed shell-session reference
 * @param agent_id Receiving-agent identifier
 * @return Owned shell delegate, or NULL on failure
 */
ar_shell_delegate_t* ar_shell_delegate__create(
    ar_log_t *ref_log,
    ar_shell_session_t *ref_session,
    int64_t agent_id);

/**
 * Destroy a shell delegate.
 * @param own_delegate Owned shell delegate
 */
void ar_shell_delegate__destroy(ar_shell_delegate_t *own_delegate);

/**
 * Get the receiving-agent identifier.
 * @param ref_delegate Borrowed shell delegate
 * @return Receiving-agent identifier, or 0 if unavailable
 */
int64_t ar_shell_delegate__get_agent_id(const ar_shell_delegate_t *ref_delegate);

/**
 * Create an input envelope for one terminal line.
 * @param ref_text Borrowed input text
 * @return Owned map envelope, or NULL on failure
 */
ar_data_t* ar_shell_delegate__create_input_envelope(const char *ref_text);

/**
 * Forward one input line to the receiving agent.
 * @param mut_delegate Mutable shell delegate
 * @param mut_system Mutable system instance
 * @param ref_text Borrowed input text
 * @return true on successful handoff, false otherwise
 */
bool ar_shell_delegate__forward_input(
    ar_shell_delegate_t *mut_delegate,
    ar_system_t *mut_system,
    const char *ref_text);

#endif /* AGERUN_SHELL_DELEGATE_H */
