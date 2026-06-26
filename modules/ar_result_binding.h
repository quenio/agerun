/**
 * @file ar_result_binding.h
 * @brief Effectful instruction result-binding module for AgeRun
 *
 * This module owns statement-level result binding for assigned effectful
 * instructions. Pure expression assignment remains handled by the assignment
 * evaluator.
 */

#ifndef AGERUN_RESULT_BINDING_H
#define AGERUN_RESULT_BINDING_H

#include <stdbool.h>
#include "ar_data.h"
#include "ar_frame.h"
#include "ar_log.h"

/**
 * Validate an effectful result-binding target before evaluating side effects.
 * @param ref_log The log instance for error reporting (borrowed reference, may be NULL)
 * @param ref_result_path The result path to validate (borrowed reference, may be NULL)
 * @return true if the target is allowed for effectful evaluation, false otherwise
 * @note A NULL path means there is no result binding and is considered valid.
 */
bool ar_result_binding__validate_target(
    ar_log_t *ref_log,
    const char *ref_result_path
);

/**
 * Bind an owned effectful result into frame memory.
 * @param ref_log The log instance for error reporting (borrowed reference, may be NULL)
 * @param ref_frame The execution frame containing mutable memory (borrowed reference)
 * @param ref_result_path The effectful result path (borrowed reference)
 * @param own_result The owned result value to bind
 * @return true if the value was stored and ownership transferred, false otherwise
 * @note Ownership: Takes ownership of own_result. On successful storage, ownership transfers to
 *       frame memory. On failed storage, own_result is destroyed.
 */
bool ar_result_binding__bind(
    ar_log_t *ref_log,
    const ar_frame_t *ref_frame,
    const char *ref_result_path,
    ar_data_t *own_result
);

#endif /* AGERUN_RESULT_BINDING_H */
