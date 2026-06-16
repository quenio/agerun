#ifndef AGERUN_PARSE_H
#define AGERUN_PARSE_H

#include "ar_data.h"

/**
 * @file ar_parse.h
 * @brief Pure parse operation shared by expression and instruction evaluators.
 */

/**
 * Create a map by parsing input with a template.
 *
 * The operation is pure: it only reads the supplied values and returns a new map.
 * Primitive arguments are interpreted as strings using the language conversion
 * rules used by template building. Non-primitive or missing arguments, malformed
 * templates, and non-matching input return an empty map.
 *
 * @param ref_template Template value to interpret as a string (borrowed reference)
 * @param ref_input Input value to interpret as a string (borrowed reference)
 * @return Newly created result map (owned by caller), or NULL on allocation failure
 * @note Ownership: Returns an owned map that caller must destroy or store.
 */
ar_data_t* ar_parse__create_result(
    const ar_data_t *ref_template,
    const ar_data_t *ref_input
);

#endif /* AGERUN_PARSE_H */
