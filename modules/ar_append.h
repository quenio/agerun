#ifndef AGERUN_APPEND_H
#define AGERUN_APPEND_H

#include "ar_data.h"

/**
 * @file ar_append.h
 * @brief Pure value-level append operation for list construction.
 */

/**
 * Create the append result for a list value and a value to append.
 *
 * The operation is pure: it only reads the supplied values and returns a new value.
 * LIST values return a new LIST containing deep copies of each source item followed
 * by a deep copy of the appended value. Missing values, non-LIST list inputs, and
 * copy failures return integer 0.
 *
 * @param ref_list List value to copy from (borrowed reference)
 * @param ref_value Value to append (borrowed reference)
 * @return Newly created result value (owned by caller), or NULL on allocation failure
 * @note Ownership: Returns an owned value that caller must destroy or store.
 */
ar_data_t* ar_append__create_result(
    const ar_data_t *ref_list,
    const ar_data_t *ref_value
);

#endif /* AGERUN_APPEND_H */
