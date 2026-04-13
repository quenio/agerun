#ifndef AGERUN_SHELL_H
#define AGERUN_SHELL_H

#include <stddef.h>
#include "ar_shell_session.h"
#include "ar_system.h"

/**
 * Opaque shell type.
 */
typedef struct ar_shell_s ar_shell_t;

/**
 * Create a shell instance.
 * @param default_mode Default acknowledgement mode
 * @return Owned shell instance, or NULL on failure
 */
ar_shell_t* ar_shell__create(ar_shell_mode_t default_mode);

/**
 * Destroy a shell instance.
 * @param own_shell Owned shell instance
 */
void ar_shell__destroy(ar_shell_t *own_shell);

/**
 * Get the shell default acknowledgement mode.
 * @param ref_shell Borrowed shell instance
 * @return Default acknowledgement mode
 */
ar_shell_mode_t ar_shell__get_default_mode(const ar_shell_t *ref_shell);

/**
 * Get the wrapped system instance.
 * @param ref_shell Borrowed shell instance
 * @return Borrowed system reference, or NULL if unavailable
 */
ar_system_t* ar_shell__get_system(const ar_shell_t *ref_shell);

/**
 * Get the number of tracked shell sessions.
 * @param ref_shell Borrowed shell instance
 * @return Session count
 */
size_t ar_shell__get_session_count(const ar_shell_t *ref_shell);

/**
 * Start one shell session.
 * @param mut_shell Mutable shell instance
 * @param mode Requested session acknowledgement mode
 * @return Borrowed session reference managed by the shell, or NULL on failure
 */
ar_shell_session_t* ar_shell__start_session(ar_shell_t *mut_shell, ar_shell_mode_t mode);

/**
 * Entry point for the `arsh` executable.
 * @param argc Argument count
 * @param argv Argument vector
 * @return Process exit code
 */
int ar_shell__main(int argc, char **argv);

#endif /* AGERUN_SHELL_H */
