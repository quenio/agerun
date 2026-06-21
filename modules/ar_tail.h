#ifndef AGERUN_TAIL_H
#define AGERUN_TAIL_H

#include "ar_data.h"

/**
 * @file ar_tail.h
 * @brief Pure tail operation shared by expression and instruction evaluators.
 */

/**
 * Create the tail result for a list value.
 *
 * The operation is pure: it only reads the supplied value and returns a new value.
 * LIST values return a new LIST containing deep copies of every item after the
 * first. Empty and single-item LIST values return a new empty LIST. Missing values,
 * non-LIST values, and copy failures return integer 0.
 *
 * @param ref_list List value to inspect (borrowed reference)
 * @return Newly created result value (owned by caller), or NULL on allocation failure
 * @note Ownership: Returns an owned value that caller must destroy or store.
 */
ar_data_t* ar_tail__create_result(
    const ar_data_t *ref_list
);

#endif /* AGERUN_TAIL_H */
