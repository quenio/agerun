#ifndef AGERUN_HEAD_H
#define AGERUN_HEAD_H

#include "ar_data.h"

/**
 * @file ar_head.h
 * @brief Pure head operation shared by expression and instruction evaluators.
 */

/**
 * Create the head result for a list value.
 *
 * The operation is pure: it only reads the supplied value and returns a new value.
 * LIST values with at least one item return a deep copy of the first item. Missing
 * values, empty LIST values, non-LIST values, and copy failures return integer 0.
 *
 * @param ref_list List value to inspect (borrowed reference)
 * @return Newly created result value (owned by caller), or NULL on allocation failure
 * @note Ownership: Returns an owned value that caller must destroy or store.
 */
ar_data_t* ar_head__create_result(
    const ar_data_t *ref_list
);

#endif /* AGERUN_HEAD_H */
