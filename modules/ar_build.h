#ifndef AGERUN_BUILD_H
#define AGERUN_BUILD_H

#include "ar_data.h"

/**
 * @file ar_build.h
 * @brief Pure build operation shared by expression and instruction evaluators.
 */

/**
 * Create a string by applying values to a template.
 *
 * The operation is pure: it only reads the supplied values and returns a new string.
 * Primitive template values are interpreted as strings. Non-primitive or missing
 * templates use an empty string fallback. Placeholder values are substituted from
 * maps when they can be converted to strings; otherwise placeholders are preserved.
 *
 * @param ref_template Template value to interpret as a string (borrowed reference)
 * @param ref_values Values map to use for placeholder substitution (borrowed reference)
 * @return Newly created result string (owned by caller), or NULL on allocation failure
 * @note Ownership: Returns an owned string that caller must destroy or store.
 */
ar_data_t* ar_build__create_result(
    const ar_data_t *ref_template,
    const ar_data_t *ref_values
);

#endif /* AGERUN_BUILD_H */
