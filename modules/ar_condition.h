#ifndef AGERUN_CONDITION_H
#define AGERUN_CONDITION_H

#include <stdbool.h>
#include "ar_data.h"

/**
 * @file ar_condition.h
 * @brief Shared condition truthiness helpers.
 */

/**
 * Determine whether a value is true under AgeRun condition semantics.
 *
 * Integer zero is false, nonzero integers are true, and non-integer or missing
 * values are false.
 *
 * @param ref_value Value to inspect (borrowed reference)
 * @return true when the value is a nonzero integer, false otherwise
 * @note Ownership: Does not take ownership of the value.
 */
bool ar_condition__is_true(
    const ar_data_t *ref_value
);

#endif /* AGERUN_CONDITION_H */
